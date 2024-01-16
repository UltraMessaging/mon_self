/* mon_self.c - Example demonstration of "stats_thread" module.
 * See https://github.com/UltraMessaging/mon_self */
/*
  (C) Copyright 2022,2023 Informatica Corporation
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
#include "stats_thread.h"

/* State to pass around. */
struct my_objs_s {
  lbm_context_t *ctx1;
  lbm_context_t *ctx2;
  lbm_rcv_t *rcv;
  lbm_src_t *src1;
  lbm_src_t *src2;
};
typedef struct my_objs_s my_objs_t;


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

  /* Stats interval 2 seconds is much too small for most producton
   * deployments, where 10 minutes or more would typically be used. */
  ENL(stats_thread1 = stats_thread_create(my_objs->ctx1, "ctx1", 2));
  stats_thread_start(stats_thread1);
  sleep(1);  /* Run the two stats threads out of phase. */
  ENL(stats_thread2 = stats_thread_create(my_objs->ctx2, "ctx2", 2));
  stats_thread_start(stats_thread2);

  E(lbm_rcv_topic_lookup(&topic_obj, my_objs->ctx1, "MyTopic", NULL));
  E(lbm_rcv_create(&my_objs->rcv, my_objs->ctx1, topic_obj, rcv_cb, my_objs, NULL));

  E(lbm_src_topic_alloc(&topic_obj, my_objs->ctx1, "MyTopic", NULL));
  E(lbm_src_create(&my_objs->src1, my_objs->ctx1, topic_obj, NULL, NULL, NULL));

  E(lbm_src_topic_alloc(&topic_obj, my_objs->ctx2, "MyTopic", NULL));
  E(lbm_src_create(&my_objs->src2, my_objs->ctx2, topic_obj, NULL, NULL, NULL));

  usleep(500000);

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
