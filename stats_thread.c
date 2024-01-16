/* stats_thread.c - Example thread to print selected UM stats.
 * See https://github.com/UltraMessaging/mon_self */
/*
  (C) Copyright 2023,2024 Informatica Corporation
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


void print_stats(stats_thread_t *stats_thread)
{
  int i, err;
  lbm_context_stats_t ctx_stats;
  char *ctx_name = stats_thread->ctx_name;
  if (ctx_name == NULL) {
    ctx_name = "";
  }

  /* Sample context stats. */
  E(lbm_context_retrieve_stats(stats_thread->ctx, &ctx_stats));

  /* Sample source stats. May require loop if our stats buffer isn't big enough. */
  int actual_src_entries;
  do {
    actual_src_entries = stats_thread->src_num_entries;
    err = lbm_context_retrieve_src_transport_stats_ex(stats_thread->ctx, &actual_src_entries,
        sizeof(lbm_src_transport_stats_t), stats_thread->src_stats);
    if (err == -1 && lbm_errnum() == LBM_EINVAL && actual_src_entries > stats_thread->src_num_entries) {
      /* We didn't allow enough space for the current transport sessions. UM gives back
       * the number of entries it needs. Increase it to allow for growth. */
      stats_thread->src_num_entries += (actual_src_entries+1)/2;
      ENL(stats_thread->src_stats = (lbm_src_transport_stats_t *)realloc(stats_thread->src_stats,
          sizeof(lbm_src_transport_stats_t) * stats_thread->src_num_entries));
    }
    else if (err == -1) {
      E(err);  /* Any other error is fatal. */
    }
  } while (err != 0);

  /* Sample receiver stats. May require loop if our stats buffer isn't big enough. */
  int actual_rcv_entries;
  do {
    actual_rcv_entries = stats_thread->rcv_num_entries;
    err = lbm_context_retrieve_rcv_transport_stats_ex(stats_thread->ctx, &actual_rcv_entries,
        sizeof(lbm_rcv_transport_stats_t), stats_thread->rcv_stats);
    if (err == -1 && lbm_errnum() == LBM_EINVAL && actual_rcv_entries > stats_thread->rcv_num_entries) {
      /* We didn't allow enough space for the current transport sessions. UM gives back
       * the number of entries it needs. Increase it to allow for growth. */
      stats_thread->rcv_num_entries += (actual_rcv_entries+1)/2;
      ENL(stats_thread->rcv_stats = (lbm_rcv_transport_stats_t *)realloc(stats_thread->rcv_stats,
          sizeof(lbm_rcv_transport_stats_t) * stats_thread->rcv_num_entries));
    }
    else if (err == -1) {
      E(err);  /* Any other error is fatal. */
    }
  } while (err != 0);

  /* Print context stats. */
  {
    unsigned long tr_drops = ctx_stats.tr_dgrams_dropped_ver + ctx_stats.tr_dgrams_dropped_type
                             + ctx_stats.tr_dgrams_dropped_malformed;
    printf("ctx_name='%s', context: tr_dgrams_sent=%lu, tr_dgrams_rcved=%lu, tr_drops=%lu"
           ", tr_src_topics=%lu, tr_rcv_topics=%lu, tr_rcv_unresolved_topics=%lu"
           ", lbtrm_unknown_msgs_rcved=%lu, lbtru_unknown_msgs_rcved=%lu"
           ", send_blocked=%lu, send_would_block=%lu, fragments_unrecoverably_lost=%lu"
           "\n",
           ctx_name, ctx_stats.tr_dgrams_sent, ctx_stats.tr_dgrams_rcved, tr_drops,
           ctx_stats.tr_src_topics, ctx_stats.tr_rcv_topics, ctx_stats.tr_rcv_unresolved_topics,
           ctx_stats.lbtrm_unknown_msgs_rcved, ctx_stats.lbtru_unknown_msgs_rcved,
           ctx_stats.send_blocked, ctx_stats.send_would_block, ctx_stats.fragments_unrecoverably_lost
    ); 
  }

  /* Print source stats, one line per published transport session. */
  for (i = 0; i < actual_src_entries; i++) {
    switch (stats_thread->src_stats[i].type) {
      case LBM_TRANSPORT_STAT_LBTRM: {
        lbm_src_transport_stats_lbtrm_t *stats = &stats_thread->src_stats[i].transport.lbtrm;
        printf("ctx_name='%s', src/lbtrm: source=%s, msgs_sent=%lu, naks_rcved=%lu"
            ", naks_ignored=%lu, naks_shed=%lu, naks_rx_delay_ignored=%lu, rxs_sent=%lu"
            "\n",
            ctx_name, stats_thread->src_stats[i].source, stats->msgs_sent, stats->naks_rcved,
            stats->naks_ignored, stats->naks_shed, stats->naks_rx_delay_ignored, stats->rxs_sent
        );
        break;
      }
      case LBM_TRANSPORT_STAT_LBTRU: {
        lbm_src_transport_stats_lbtru_t *stats = &stats_thread->src_stats[i].transport.lbtru;
        printf("ctx_name='%s', src/lbtru: source=%s, msgs_sent=%lu, naks_rcved=%lu"
            ", naks_ignored=%lu, naks_shed=%lu, naks_rx_delay_ignored=%lu, rxs_sent=%lu"
            "\n",
            ctx_name, stats_thread->src_stats[i].source, stats->msgs_sent, stats->naks_rcved,
            stats->naks_ignored, stats->naks_shed, stats->naks_rx_delay_ignored, stats->rxs_sent
        );
        break;
      }
      case LBM_TRANSPORT_STAT_TCP: {
        lbm_src_transport_stats_tcp_t *stats = &stats_thread->src_stats[i].transport.tcp;
        printf("ctx_name='%s', src/tcp: source=%s, num_clients=%lu"
            "\n",
            ctx_name, stats_thread->src_stats[i].source, stats->num_clients
        );
        break;
      }
      case LBM_TRANSPORT_STAT_LBTIPC: {
        lbm_src_transport_stats_lbtipc_t *stats = &stats_thread->src_stats[i].transport.lbtipc;
        printf("ctx_name='%s', src/lbtipc: source=%s, num_clients=%lu, msgs_sent=%lu"
            "\n",
            ctx_name, stats_thread->src_stats[i].source, stats->num_clients, stats->msgs_sent
        );
        break;
      }
      case LBM_TRANSPORT_STAT_LBTSMX: {
        lbm_src_transport_stats_lbtsmx_t *stats = &stats_thread->src_stats[i].transport.lbtsmx;
        printf("ctx_name='%s', src/lbtsmx: source=%s, num_clients=%lu, msgs_sent=%lu"
            "\n",
            ctx_name, stats_thread->src_stats[i].source, stats->num_clients, stats->msgs_sent
        );
        break;
      }
      default:
        printf("WARNING: ctx_name='%s', unrecognized transport type (%u)\n", ctx_name, stats_thread->rcv_stats[i].type);
    }  /* switch */
  }  /* for */

  /* Print receiver stats, one line per subscribed transport session. */
  for (i = 0; i < actual_rcv_entries; i++) {
    switch (stats_thread->rcv_stats[i].type) {
      case LBM_TRANSPORT_STAT_LBTRM: {
        lbm_rcv_transport_stats_lbtrm_t *stats = &stats_thread->rcv_stats[i].transport.lbtrm;
        unsigned long drops = stats->dgrams_dropped_size + stats->dgrams_dropped_type + stats->dgrams_dropped_version
                              + stats->dgrams_dropped_hdr + stats->dgrams_dropped_other;
        printf("ctx_name='%s', rcv/lbtrm: source=%s, msgs_rcved=%lu, naks_sent=%lu"
               ", lost=%lu, unrecovered_txw=%lu, unrecovered_tmo=%lu, lbm_msgs_rcved=%lu"
               ", lbm_msgs_no_topic_rcved=%lu, drops=%lu, out_of_order=%lu"
               "\n",
               ctx_name, stats_thread->rcv_stats[i].source, stats->msgs_rcved, stats->naks_sent,
               stats->lost, stats->unrecovered_txw, stats->unrecovered_tmo, stats->lbm_msgs_rcved,
               drops, stats->lbm_msgs_no_topic_rcved, stats->out_of_order
        );
        break;
      }
      case LBM_TRANSPORT_STAT_LBTRU: {
        lbm_rcv_transport_stats_lbtru_t *stats = &stats_thread->rcv_stats[i].transport.lbtru;
        unsigned long drops = stats->dgrams_dropped_size + stats->dgrams_dropped_type + stats->dgrams_dropped_version
                              + stats->dgrams_dropped_hdr + stats->dgrams_dropped_sid + stats->dgrams_dropped_other;
        printf("ctx_name='%s', rcv/lbtru: source=%s, msgs_rcved=%lu, naks_sent=%lu"
               ", lost=%lu, unrecovered_txw=%lu, unrecovered_tmo=%lu, lbm_msgs_rcved=%lu"
               ", lbm_msgs_no_topic_rcved=%lu, drops=%lu"
               "\n",
               ctx_name, stats_thread->rcv_stats[i].source, stats->msgs_rcved, stats->naks_sent,
               stats->lost, stats->unrecovered_txw, stats->unrecovered_tmo, stats->lbm_msgs_rcved,
               drops, stats->lbm_msgs_no_topic_rcved
        );
        break;
      }
      case LBM_TRANSPORT_STAT_TCP: {
        lbm_rcv_transport_stats_tcp_t *stats = &stats_thread->rcv_stats[i].transport.tcp;
        printf("ctx_name='%s', rcv/tcp: source=%s, lbm_msgs_rcved=%lu, lbm_msgs_no_topic_rcved=%lu"
            "\n",
            ctx_name, stats_thread->rcv_stats[i].source, stats->lbm_msgs_rcved, stats->lbm_msgs_no_topic_rcved
        );
        break;
      }
      case LBM_TRANSPORT_STAT_LBTIPC: {
        lbm_rcv_transport_stats_lbtipc_t *stats = &stats_thread->rcv_stats[i].transport.lbtipc;
        printf("ctx_name='%s', rcv/lbtipc: source=%s, msgs_rcved=%lu, lbm_msgs_rcved=%lu, lbm_msgs_no_topic_rcved=%lu"
            "\n",
            ctx_name, stats_thread->rcv_stats[i].source, stats->msgs_rcved, stats->lbm_msgs_rcved, stats->lbm_msgs_no_topic_rcved
        );
        break;
      }
      case LBM_TRANSPORT_STAT_LBTSMX: {
        lbm_rcv_transport_stats_lbtsmx_t *stats = &stats_thread->rcv_stats[i].transport.lbtsmx;
        printf("ctx_name='%s', rcv/lbtsmx: source=%s, msgs_rcved=%lu, lbm_msgs_rcved=%lu, lbm_msgs_no_topic_rcved=%lu"
            "\n",
            ctx_name, stats_thread->rcv_stats[i].source, stats->msgs_rcved, stats->lbm_msgs_rcved, stats->lbm_msgs_no_topic_rcved
        );
        break;
      }
      default:
        printf("WARNING: ctx_name='%s', unrecognized transport type (%u)\n", ctx_name, stats_thread->rcv_stats[i].type);
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
  stats_thread->rcv_num_entries = 100;
  ENL(stats_thread->rcv_stats = (lbm_rcv_transport_stats_t *)malloc(sizeof(lbm_rcv_transport_stats_t)
                                                                    * stats_thread->rcv_num_entries));
  stats_thread->src_num_entries = 100;
  ENL(stats_thread->src_stats = (lbm_src_transport_stats_t *)malloc(sizeof(lbm_src_transport_stats_t)
                                                                    * stats_thread->src_num_entries));

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

  /* Clean up. */
  if (stats_thread->ctx_name != NULL) {
    free(stats_thread->ctx_name);
  }
  free(stats_thread->rcv_stats);
  free(stats_thread->src_stats);
  free(stats_thread);
}  /* stats_thread_delete */
