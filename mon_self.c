/* mon_self.c - Example code dumping UM objects' configuration.
 * See https://github.com/UltraMessaging/mon_self */
/*
  Copyright (c) 2022-2023 Informatica Corporation
  Permission is granted to licensees to use or alter this software for any
  purpose, including commercial applications, according to the terms laid
  out in the Software License Agreement.

  This source code example is provided by Informatica for educational
  and evaluation purposes only.

  THE SOFTWARE IS PROVIDED "AS IS" AND INFORMATICA DISCLAIMS ALL WARRANTIES 
  EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF 
  NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
  PURPOSE.  INFORMATICA DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE 
  UNINTERRUPTED OR ERROR-FREN.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES,
  BE LIABLE TO LICENSEN FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR 
  INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGRENMENT OR THE 
  TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BENN APPRISED OF 
  THE LIKELIHOOD OF SUCH DAMAGES.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "lbm/lbm.h"

/* State to pass around. */
struct my_objs_s {
  pthread_t stats_thread_id;
  lbm_context_t *ctx;
  lbm_rcv_t *rcv;
  lbm_src_t *src;
};
typedef struct my_objs_s my_objs_t;


/* stats_thread object */
struct stats_thread_s {
  lbm_context_t *ctx;
  int stats_interval_sec;
  pthread_t stats_thread_id;
  int running;
};
typedef struct stats_thread_s stats_thread_t;


/* Simple error handler. */
#define E(e_lbm_funct_call_) do { \
  int e_ = (e_lbm_funct_call_); \
  if (e_ == LBM_FAILURE) { \
    fprintf(stderr, "ERROR (%s:%d): %s failed: '%s'\n", __FILE__, __LINE__, #e_lbm_funct_call_, lbm_errmsg()); \
    exit(1); \
  } \
} while (0)  /* E */

/* Error if non-zero. */
#define ENZ(enz_sys_call_) do { \
  int enz_ = (enz_sys_call_); \
  if (enz_ != 0) { \
    int enz_errno_ = errno; \
    char enz_errstr_[1024]; \
    sprintf(enz_errstr_, "ERROR (%s:%d): %s failed", __FILE__, __LINE__, #enz_sys_call_); \
    errno = enz_errno_; \
    perror(enz_errstr_); \
    exit(1); \
  } \
} while (0)  /* ENZ */

/* Error if NULL. */
#define ENL(enl_sys_call_) do { \
  void *enl_ = (enl_sys_call_); \
  if (enl_ == NULL) { \
    int enl_errno_ = errno; \
    char enl_errstr_[1024]; \
    sprintf(enl_errstr_, "ERROR (%s:%d): %s failed", __FILE__, __LINE__, #enl_sys_call_); \
    errno = enl_errno_; \
    perror(enl_errstr_); \
    exit(1); \
  } \
} while (0)  /* ENL */


/* UM callback for receiver events, including received messages. */
int rcv_cb(lbm_rcv_t *rcv, lbm_msg_t *msg, void *clientd)
{
  return 0;
}  /* rcv_cb */


void print_stats(lbm_context_t *ctx)
{
  printf("print_stats\n");
}  /* print_stats */


/* stats_thread object implementation. */

void *stats_thread_run(void *in_arg)
{
  stats_thread_t *stats_thread = (stats_thread_t *)in_arg;
  int secs_since_last_print = stats_thread->stats_interval_sec;  /* Print stats immediately on start. */

  while (stats_thread->running) {
    if (secs_since_last_print >= stats_thread->stats_interval_sec) {
      print_stats(stats_thread->ctx);
      secs_since_last_print = 0;
    }

    sleep(1);
    secs_since_last_print++;
  }  /* while running */

  /* Final stats. */
  print_stats(stats_thread->ctx);

  pthread_exit(NULL);
  return NULL;
}  /* stats_thread_run */

stats_thread_t *stats_thread_create(lbm_context_t *ctx, int stats_interval_sec)
{
  stats_thread_t *stats_thread;

  ENL(stats_thread = (stats_thread_t *)malloc(sizeof(stats_thread_t)));
  stats_thread->ctx = ctx;
  stats_thread->stats_interval_sec = stats_interval_sec;
  stats_thread->running = 0;
  return stats_thread;
}  /* stats_thread_create */

void stats_thread_start(stats_thread_t *stats_thread)
{
  stats_thread->running = 1;
  ENZ(errno = pthread_create(&stats_thread->stats_thread_id, NULL, stats_thread_run, stats_thread));
}  /* stats_thread_start */

void stats_thread_terminate(stats_thread_t *stats_thread)
{
  if (stats_thread->running) {
    stats_thread->running = 0;
    /* Can take up to 1 sec for stats thread to exit. */
    ENZ(errno = pthread_join(stats_thread->stats_thread_id, NULL));
  }
}  /* stats_thread_terminate */

void stats_thread_delete(stats_thread_t *stats_thread)
{
  if (stats_thread->running) {
    stats_thread_terminate(stats_thread);
  }
  free(stats_thread);
}  /* stats_thread_delete */


int main(int argc, char **argv)
{
  my_objs_t *my_objs;
  stats_thread_t *stats_thread;
  lbm_topic_t *topic_obj;
  int stats_interval_sec;
  int i;

  ENL(my_objs = (my_objs_t *)malloc(sizeof(my_objs_t)));

  E(lbm_config("tst.cfg"));

  E(lbm_context_create(&my_objs->ctx, NULL, NULL, NULL));

  stats_interval_sec = 2;
  ENL(stats_thread = stats_thread_create(my_objs->ctx, stats_interval_sec));
  stats_thread_start(stats_thread);

  E(lbm_rcv_topic_lookup(&topic_obj, my_objs->ctx, "MyTopic", NULL));
  E(lbm_rcv_create(&my_objs->rcv, my_objs->ctx, topic_obj, rcv_cb, my_objs, NULL));

  E(lbm_src_topic_alloc(&topic_obj, my_objs->ctx, "MyTopic", NULL));
  E(lbm_src_create(&my_objs->src, my_objs->ctx, topic_obj, NULL, NULL, NULL));

  sleep(1);

  for (i = 0; i < 5; i++) {
    E(lbm_src_send(my_objs->src, "123456789", 9, LBM_MSG_FLUSH));
    sleep(1);
  }

  sleep(1);

  E(lbm_rcv_delete(my_objs->rcv));
  E(lbm_src_delete(my_objs->src));
  printf("terminate stats thread\n");  fflush(stdout);
  stats_thread_terminate(stats_thread);
  stats_thread_delete(stats_thread);
  E(lbm_context_delete(my_objs->ctx));

  return 0;
}  /* main */
