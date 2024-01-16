/* StatsThread.java - Example thread to print selected UM stats.
 * See https://github.com/UltraMessaging/cfg_dump */
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
  UNINTERRUPTED OR ERROR-FREE.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES,
  BE LIABLE TO LICENSEE FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR 
  INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGREEMENT OR THE 
  TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BEEN APPRISED OF 
  THE LIKELIHOOD OF SUCH DAMAGES.
*/

import java.util.*;
import java.nio.*;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.atomic.AtomicInteger;
import com.latencybusters.lbm.*;


class StatsThread implements Runnable {
  private int statsIntervalSec;
  private LBMContext ctx;
  private String ctxName;
  private Thread thisThread;
  private boolean running;
  private LBMContextStatistics ctxStats;
  private int rcvNumEntries;
  private LBMReceiverStatistics rcvStats;
  private int srcNumEntries;
  private LBMSourceStatistics srcStats;

  public StatsThread(LBMContext ctx, String ctxName, int statsIntervalSec) {
    this.statsIntervalSec = statsIntervalSec;
    this.ctx = ctx;
    this.ctxName = ctxName;
    thisThread = null;
    running = false;
    // Fields used by context stats.
    ctxStats = null;
    // Fields used by receive stats.
    rcvNumEntries = 100;
    rcvStats = null;
    // Fields used by source stats.
    srcNumEntries = 100;
    srcStats = null;
  }

  public void start() {
    running = true;
    thisThread = new Thread(this);
    thisThread.start();
  }

  public void terminate() {
    if (running) {
      running = false;
      try {
        thisThread.join();
      } catch (Exception e) {
        System.err.println("Error: join: " + e.toString());
      }
    }
  }  // terminate

  public void run() {
    int secsSinceLastPrint = statsIntervalSec;  // Print stats immediately on start.
    while (running) {
      if (secsSinceLastPrint >= statsIntervalSec) {
        try {
          printStats();
        } catch (Exception e) {
          System.err.println("Error: printStats: " + e.toString());
          running = false;  // print error not recoverable.
        }
        secsSinceLastPrint = 0;
      }
      try { Thread.sleep(1000); } catch (Exception e) {}
      secsSinceLastPrint++;
    }  // while running

    // Final stats.
    try {
      printStats();
    } catch (Exception e) {
      System.err.println("Error: printStats: " + e.toString());
      running = false;  // print error not recoverable.
    }
  }  // run

  public void printStats() throws Exception {
    if (ctxStats == null) {
      // First time here, get a new stats object.
      ctxStats = ctx.getStatistics();
    }
    else {
      ctxStats.refresh(ctx);
    }

    boolean haveSrcStats = false;
    while (!haveSrcStats) {
      if (srcStats == null) {
        // First time here, get a new stats object.
        try {
          srcStats = ctx.getSourceStatistics(srcNumEntries);
          haveSrcStats = true;
        } catch (LBMException ex){
          srcNumEntries *= 2;
        }
      }
      else {  // srcStats not null
        // Have stats object, just refresh (avoids garbage).
        try {
          srcStats.refresh(ctx, srcNumEntries);
          haveSrcStats = true;
        } catch (LBMException ex){
          srcNumEntries *= 2;
        }
      }
    }  // while not haveSrcStats

    boolean haveRcvStats = false;
    while (!haveRcvStats) {
      if (rcvStats == null) {
        // First time here, get a new stats object.
        try {
          rcvStats = ctx.getReceiverStatistics(rcvNumEntries);
          haveRcvStats = true;
        } catch (LBMException ex){
          rcvNumEntries *= 2;
        }
      }
      else {  // rcvStats not null
        // Have stats object, just refresh (avoids garbage).
        try {
          rcvStats.refresh(ctx, rcvNumEntries);
          haveRcvStats = true;
        } catch (LBMException ex){
          rcvNumEntries *= 2;
        }
      }
    }  // while not haveRcvStats

    // Print context stats.
    //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__context__stats__t__stct.html
    long trDrops = ctxStats.topicResolutionDatagramsDroppedVersion()
                  + ctxStats.topicResolutionDatagramsDroppedType()
                  + ctxStats.topicResolutionDatagramsDroppedMalformed();
    System.out.println("ctx_name='" + ctxName + "', context:"
                       + " tr_dgrams_sent=" + ctxStats.topicResolutionDatagramsSent()
                       + ", tr_dgrams_rcved=" + ctxStats.topicResolutionDatagramsReceived()
                       + ", tr_drops=" + trDrops
                       + ", tr_src_topics=" + ctxStats.topicResolutionSourceTopics()
                       + ", tr_rcv_topics=" + ctxStats.topicResolutionReceiverTopics()
                       + ", tr_rcv_unresolved_topics=" + ctxStats.topicResolutionUnresolvedReceiverTopics()
                       + ", lbtrm_unknown_msgs_rcved=" + ctxStats.lbtrmUnknownMessagesReceived()
                       + ", lbtru_unknown_msgs_rcved=" + ctxStats.lbtruUnknownMessagesReceived()
                       + ", send_blocked=" + ctxStats.sendBlocked()
                       + ", send_would_block=" + ctxStats.sendWouldBlock()
                       + ", fragments_unrecoverably_lost=" + ctxStats.fragmentsUnrecoverablyLost()
    );

    // Print source stats.
    for (int i = 0; i < srcStats.size(); i++) {
      switch (srcStats.type(i)) {
        case LBM.TRANSPORT_STAT_LBTRM: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__lbtrm__t__stct.html
          System.out.println("ctx_name='" + ctxName + "', src/lbtrm: source=" + srcStats.source(i)
                             + ", msgs_sent=" + srcStats.messagesSent(i)
                             + ", naks_rcved=" + srcStats.naksReceived(i)
                             + ", naks_ignored=" + srcStats.naksIgnored(i)
                             + ", naks_shed=" + srcStats.naksShed(i)
                             + ", naks_rx_delay_ignored=" + srcStats.naksIgnoredRetransmitDelay(i)
                             + ", rxs_sent=" + srcStats.retransmissionsSent(i)
          );
          break;
        }
        case LBM.TRANSPORT_STAT_LBTRU: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__lbtru__t__stct.html
          System.out.println("ctx_name='" + ctxName + "', src/lbtru: source=" + srcStats.source(i)
                             + ", msgs_sent=" + srcStats.messagesSent(i)
                             + ", naks_rcved=" + srcStats.naksReceived(i)
                             + ", naks_ignored=" + srcStats.naksIgnored(i)
                             + ", naks_shed=" + srcStats.naksShed(i)
                             + ", naks_rx_delay_ignored=" + srcStats.naksIgnoredRetransmitDelay(i)
                             + ", rxs_sent=" + srcStats.retransmissionsSent(i)
          );
          break;
        }
        case LBM.TRANSPORT_STAT_TCP: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__tcp__t__stct.html
          System.out.println("ctx_name='" + ctxName + "', src/tcp: source=" + srcStats.source(i)
                             + ", num_clients=" + srcStats.numberOfClients(i)
          );
          break;
        }
        case LBM.TRANSPORT_STAT_LBTIPC: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__lbtipc__t__stct.html
          System.out.println("ctx_name='" + ctxName + "', src/lbtipc: source=" + srcStats.source(i)
                             + ", num_clients=" + srcStats.numberOfClients(i)
                             + ", msgs_sent=" + srcStats.messagesSent(i)
          );
          break;
        }
        case LBM.TRANSPORT_STAT_LBTSMX: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__lbtsmx__t__stct.html
          System.out.println("ctx_name='" + ctxName + "', src/lbtsmx: source=" + srcStats.source(i)
                             + ", num_clients=" + srcStats.numberOfClients(i)
                             + ", msgs_sent=" + srcStats.messagesSent(i)
          );
          break;
        }
      }  // switch
    }  // for i

    // Print receiver stats.
    for (int i = 0; i < rcvStats.size(); i++) {
      switch (rcvStats.type(i)) {
        case LBM.TRANSPORT_STAT_LBTRM: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__lbtrm__t__stct.html
          long drops = rcvStats.datagramsDroppedIncorrectSize(i) + rcvStats.datagramsDroppedType(i)
                      + rcvStats.datagramsDroppedVersion(i) + rcvStats.datagramsDroppedHeader(i)
                      + rcvStats.datagramsDroppedOther(i);
          System.out.println("ctx_name='" + ctxName + "', rcv/lbtrm: source=" + rcvStats.source(i)
                             + ", msgs_rcved=" + rcvStats.messagesReceived(i)
                             + ", naks_sent=" + rcvStats.naksSent(i)
                             + ", lost=" + rcvStats.lost(i)
                             + ", unrecovered_txw=" + rcvStats.unrecoveredMessagesWindowAdvance(i)
                             + ", unrecovered_tmo=" + rcvStats.unrecoveredMessagesNakGenerationTimeout(i)
                             + ", lbm_msgs_rcved=" + rcvStats.lbmMessagesReceived(i)
                             + ", lbm_msgs_no_topic_rcved=" + rcvStats.noTopicMessagesReceived(i)
                             + ", drops=" + drops
                             + ", out_of_order=" + rcvStats.outOfOrder(i)
          );
          break;
        }
        case LBM.TRANSPORT_STAT_LBTRU: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__lbtru__t__stct.html
          long drops = rcvStats.datagramsDroppedIncorrectSize(i) + rcvStats.datagramsDroppedType(i)
                      + rcvStats.datagramsDroppedVersion(i) + rcvStats.datagramsDroppedHeader(i)
                      + rcvStats.datagramsDroppedOther(i) + rcvStats.datagramsDroppedSID(i);
          System.out.println("ctx_name='" + ctxName + "', rcv/lbtru: source=" + rcvStats.source(i)
                             + ", msgs_rcved=" + rcvStats.messagesReceived(i)
                             + ", naks_sent=" + rcvStats.naksSent(i)
                             + ", lost=" + rcvStats.lost(i)
                             + ", unrecovered_txw=" + rcvStats.unrecoveredMessagesWindowAdvance(i)
                             + ", unrecovered_tmo=" + rcvStats.unrecoveredMessagesNakGenerationTimeout(i)
                             + ", lbm_msgs_rcved=" + rcvStats.lbmMessagesReceived(i)
                             + ", lbm_msgs_no_topic_rcved=" + rcvStats.noTopicMessagesReceived(i)
                             + ", drops=" + drops
          );
          break;
        }
        case LBM.TRANSPORT_STAT_TCP: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__tcp__t__stct.html
          System.out.println("ctx_name='" + ctxName + "', rcv/tcp: source=" + rcvStats.source(i)
                             + ", lbm_msgs_rcved=" + rcvStats.lbmMessagesReceived(i)
                             + ", lbm_msgs_no_topic_rcved=" + rcvStats.noTopicMessagesReceived(i)
          );
          break;
        }
        case LBM.TRANSPORT_STAT_LBTIPC: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__lbtipc__t__stct.html
          System.out.println("ctx_name='" + ctxName + "', rcv/lbtipc: source=" + rcvStats.source(i)
                             + ", msgs_rcved=" + rcvStats.messagesReceived(i)
                             + ", lbm_msgs_rcved=" + rcvStats.lbmMessagesReceived(i)
                             + ", lbm_msgs_no_topic_rcved=" + rcvStats.noTopicMessagesReceived(i)
          );
          break;
        }
        case LBM.TRANSPORT_STAT_LBTSMX: {
          //****** See https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__lbtsmx__t__stct.html
          System.out.println("ctx_name='" + ctxName + "', rcv/lbtsmx: source=" + rcvStats.source(i)
                             + ", msgs_rcved=" + rcvStats.messagesReceived(i)
                             + ", lbm_msgs_rcved=" + rcvStats.lbmMessagesReceived(i)
                             + ", lbm_msgs_no_topic_rcved=" + rcvStats.noTopicMessagesReceived(i)
          );
          break;
        }
      }  // switch
    }  // for i

  }  // printStats
} // StatsThread
