/* stats_thread.c - public definitions for stats_thread module.
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

#ifndef STATS_THREAD_H
#define STATS_THREAD_H

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "lbm/lbm.h"


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


stats_thread_t *stats_thread_create(lbm_context_t *ctx, char *ctx_name, int stats_interval_sec);
void stats_thread_start(stats_thread_t *stats_thread);
void stats_thread_terminate(stats_thread_t *stats_thread);
void stats_thread_delete(stats_thread_t *stats_thread);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* STATS_THREAD_H */
