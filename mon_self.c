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
  lbm_context_t *ctx1;
  lbm_context_t *ctx2;
  lbm_rcv_t *rcv;
  lbm_src_t *src1;
  lbm_src_t *src2;
};
typedef struct my_objs_s my_objs_t;


/* stats_thread object */
struct stats_thread_s {
  lbm_context_t *ctx;
  char *ctx_name;
  int stats_interval_sec;
  pthread_t stats_thread_id;
  int running;
  /* Fields used by receive stats. */
  int rcv_num_entries;
  lbm_rcv_transport_stats_t *rcv_stats;
  /* Fields used by source stats. */
  int src_num_entries;
  lbm_src_transport_stats_t *src_stats;
};
typedef struct stats_thread_s stats_thread_t;


/* Simple error handler for LBM. */
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


void print_stats(stats_thread_t *stats_thread)
{
  int i, err;
  int rtn_entries;
  char *ctx_name = stats_thread->ctx_name;
  if (ctx_name == NULL) {
    ctx_name = "";
  }

  /* Sample stats. May require loop if our stats buffer isn't big enough. */
  do {
    rtn_entries = stats_thread->rcv_num_entries;
    err = lbm_context_retrieve_rcv_transport_stats_ex(stats_thread->ctx, &rtn_entries,
        sizeof(lbm_rcv_transport_stats_t), stats_thread->rcv_stats);
    if (err == -1 && lbm_errnum() == LBM_EINVAL && rtn_entries > stats_thread->rcv_num_entries) {
      /* We didn't allow enough space for the current transport sessions. UM gives back
       * the number of entries it needs. Increase it to allow for growth. */
      stats_thread->rcv_num_entries += (rtn_entries+1)/2;
      ENL(stats_thread->rcv_stats = (lbm_rcv_transport_stats_t *)realloc(stats_thread->rcv_stats,
          sizeof(lbm_rcv_transport_stats_t) * stats_thread->rcv_num_entries));
    }
    else if (err == -1) {
      E(err);  /* Any other error is fatal. */
    }
  } while (err != 0);

  for (i = 0; i < stats_thread->rcv_num_entries; i++) {
    switch (stats_thread->rcv_stats[i].type) {
      case LBM_TRANSPORT_STAT_LBTRM: {
        lbm_rcv_transport_stats_lbtrm_t *stats = &stats_thread->rcv_stats[i].transport.lbtrm;
        unsigned long drops = stats->dgrams_dropped_size + stats->dgrams_dropped_type + stats->dgrams_dropped_version
                              + stats->dgrams_dropped_hdr + stats->dgrams_dropped_other;
        printf("ctx_name='%s', rcv/lbtrm: source=%s, msgs_rcved=%lu, naks_sent=%lu"
               ", lost=%lu, unrecovered_txw=%lu, unrecovered_tmo=%lu, lbm_msgs_rcved=%lu"
               ", lbm_msgs_no_topic_rcved=%lu, drops=%lu, out_of_order=%lu\n",
               ctx_name, stats_thread->rcv_stats[i].source, stats->msgs_rcved, stats->naks_sent,
               stats->lost, stats->unrecovered_txw, stats->unrecovered_tmo, stats->lbm_msgs_rcved,
               drops, stats->lbm_msgs_no_topic_rcved, stats->out_of_order);
        break;
      }
      case LBM_TRANSPORT_STAT_LBTRU: {
        lbm_rcv_transport_stats_lbtru_t *stats = &stats_thread->rcv_stats[i].transport.lbtru;
        unsigned long drops = stats->dgrams_dropped_size + stats->dgrams_dropped_type + stats->dgrams_dropped_version
                              + stats->dgrams_dropped_hdr + stats->dgrams_dropped_other;
        printf("ctx_name='%s', rcv/lbtru: source=%s, msgs_rcved=%lu, naks_sent=%lu"
               ", lost=%lu, unrecovered_txw=%lu, unrecovered_tmo=%lu, lbm_msgs_rcved=%lu"
               ", lbm_msgs_no_topic_rcved=%lu, drops=%lu\n",
               ctx_name, stats_thread->rcv_stats[i].source, stats->msgs_rcved, stats->naks_sent,
               stats->lost, stats->unrecovered_txw, stats->unrecovered_tmo, stats->lbm_msgs_rcved,
               drops, stats->lbm_msgs_no_topic_rcved);
        break;
      }
    }  /* switch */
  }  /* for */

  /* Sample stats. May require loop if our stats buffer isn't big enough. */
  do {
    rtn_entries = stats_thread->src_num_entries;
    err = lbm_context_retrieve_src_transport_stats_ex(stats_thread->ctx, &rtn_entries,
        sizeof(lbm_src_transport_stats_t), stats_thread->src_stats);
    if (err == -1 && lbm_errnum() == LBM_EINVAL && rtn_entries > stats_thread->src_num_entries) {
      /* We didn't allow enough space for the current transport sessions. UM gives back
       * the number of entries it needs. Increase it to allow for growth. */
      stats_thread->src_num_entries += (rtn_entries+1)/2;
      ENL(stats_thread->src_stats = (lbm_src_transport_stats_t *)realloc(stats_thread->src_stats,
          sizeof(lbm_src_transport_stats_t) * stats_thread->src_num_entries));
    }
    else if (err == -1) {
      E(err);  /* Any other error is fatal. */
    }
  } while (err != 0);

  for (i = 0; i < stats_thread->src_num_entries; i++) {
    switch (stats_thread->src_stats[i].type) {
      case LBM_TRANSPORT_STAT_LBTRM: {
        lbm_src_transport_stats_lbtrm_t *stats = &stats_thread->src_stats[i].transport.lbtrm;
        printf("ctx_name='%s', src/lbtrm: source=%s, msgs_sent=%lu, naks_rcved=%lu"
            ", naks_ignored=%lu, naks_shed=%lu, naks_rx_delay_ignored=%lu, rxs_sent=%lu\n",
            ctx_name, stats_thread->src_stats[i].source, stats->msgs_sent, stats->naks_rcved,
            stats->naks_ignored, stats->naks_shed, stats->naks_rx_delay_ignored, stats->rxs_sent);
        break;
      }
      case LBM_TRANSPORT_STAT_LBTRU: {
        lbm_src_transport_stats_lbtru_t *stats = &stats_thread->src_stats[i].transport.lbtru;
        printf("ctx_name='%s', src/lbtru: source=%s, msgs_sent=%lu, naks_rcved=%lu"
            ", naks_ignored=%lu, naks_shed=%lu, naks_rx_delay_ignored=%lu, rxs_sent=%lu\n",
            ctx_name, stats_thread->src_stats[i].source, stats->msgs_sent, stats->naks_rcved,
            stats->naks_ignored, stats->naks_shed, stats->naks_rx_delay_ignored, stats->rxs_sent);
        break;
      }
    }  /* switch */
  }  /* for */

}  /* print_stats */


/* stats_thread object implementation. */

void *stats_thread_run(void *in_arg)
{
  stats_thread_t *stats_thread = (stats_thread_t *)in_arg;
  int secs_since_last_print = stats_thread->stats_interval_sec;  /* Print stats immediately on start. */

  while (stats_thread->running) {
    if (secs_since_last_print >= stats_thread->stats_interval_sec) {
      print_stats(stats_thread);
      secs_since_last_print = 0;
    }

    sleep(1);
    secs_since_last_print++;
  }  /* while running */

  /* Final stats. */
  print_stats(stats_thread);

  pthread_exit(NULL);
  return NULL;
}  /* stats_thread_run */

stats_thread_t *stats_thread_create(lbm_context_t *ctx, char *ctx_name, int stats_interval_sec)
{
  stats_thread_t *stats_thread;

  ENL(stats_thread = (stats_thread_t *)malloc(sizeof(stats_thread_t)));
  stats_thread->ctx = ctx;
  if (ctx_name != NULL) {
    ENL(stats_thread->ctx_name = strdup(ctx_name));
  } else {
    stats_thread->ctx_name = NULL;
  }
  stats_thread->stats_interval_sec = stats_interval_sec;
  stats_thread->running = 0;
  stats_thread->rcv_num_entries = 1;
  ENL(stats_thread->rcv_stats = (lbm_rcv_transport_stats_t *)malloc(sizeof(lbm_rcv_transport_stats_t) * stats_thread->rcv_num_entries));
  stats_thread->src_num_entries = 1;
  ENL(stats_thread->src_stats = (lbm_src_transport_stats_t *)malloc(sizeof(lbm_src_transport_stats_t) * stats_thread->src_num_entries));

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
  if (stats_thread->ctx_name != NULL) {
    free(stats_thread->ctx_name);
  }
  free(stats_thread);
}  /* stats_thread_delete */


int main(int argc, char **argv)
{
  my_objs_t *my_objs;
  stats_thread_t *stats_thread1;
  stats_thread_t *stats_thread2;
  lbm_topic_t *topic_obj;
  int i;

  ENL(my_objs = (my_objs_t *)malloc(sizeof(my_objs_t)));

  E(lbm_config("tst.cfg"));

  E(lbm_context_create(&my_objs->ctx1, NULL, NULL, NULL));
  E(lbm_context_create(&my_objs->ctx2, NULL, NULL, NULL));

  /* Run two stats threads out of sync. */
  ENL(stats_thread1 = stats_thread_create(my_objs->ctx1, "ctx1", 2));
  stats_thread_start(stats_thread1);
  sleep(1);
  ENL(stats_thread2 = stats_thread_create(my_objs->ctx2, "ctx2", 2));
  stats_thread_start(stats_thread2);

  E(lbm_rcv_topic_lookup(&topic_obj, my_objs->ctx1, "MyTopic", NULL));
  E(lbm_rcv_create(&my_objs->rcv, my_objs->ctx1, topic_obj, rcv_cb, my_objs, NULL));

  E(lbm_src_topic_alloc(&topic_obj, my_objs->ctx1, "MyTopic", NULL));
  E(lbm_src_create(&my_objs->src1, my_objs->ctx1, topic_obj, NULL, NULL, NULL));

  E(lbm_src_topic_alloc(&topic_obj, my_objs->ctx2, "MyTopic", NULL));
  E(lbm_src_create(&my_objs->src2, my_objs->ctx2, topic_obj, NULL, NULL, NULL));

  sleep(1);

  for (i = 0; i < 5; i++) {
    E(lbm_src_send(my_objs->src1, "123456789", 9, LBM_MSG_FLUSH));
    E(lbm_src_send(my_objs->src2, "123456789", 9, LBM_MSG_FLUSH));
    sleep(1);
  }

  sleep(1);

  E(lbm_rcv_delete(my_objs->rcv));
  E(lbm_src_delete(my_objs->src1));
  E(lbm_src_delete(my_objs->src2));
  printf("terminate stats thread1\n");  fflush(stdout);
  stats_thread_terminate(stats_thread1);
  stats_thread_delete(stats_thread1);
  printf("terminate stats thread2\n");  fflush(stdout);
  stats_thread_terminate(stats_thread2);
  stats_thread_delete(stats_thread2);
  E(lbm_context_delete(my_objs->ctx1));
  E(lbm_context_delete(my_objs->ctx2));

  return 0;
}  /* main */
