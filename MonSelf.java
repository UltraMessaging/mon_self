/* MonSelf.java - performance measurement tool.
 * See https://github.com/UltraMessaging/cfg_dump */
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

// The application class supplies the onReceive, onSourceEvent,
// onTransportMapping, and LBMLog callbacks directly.
class MonSelf implements LBMReceiverCallback {

  public static void main(String[] args) throws Exception {
    // The body of the program is in the "run" method.
    MonSelf application = new MonSelf();
    application.run(args);
  }  // main


  private void run(String[] args) throws Exception {
    int i;
    LBM lbm = new LBM();
    int myStatsIntervalSec = 2;

    LBM.setConfiguration("tst.cfg");

    LBMContext myCtx = new LBMContext();

    StatsThread myStatsThread = new StatsThread(myCtx, myStatsIntervalSec);
    myStatsThread.start();

    LBMTopic topicObj = myCtx.lookupTopic("MyTopic");
    LBMReceiver myRcv = myCtx.createReceiver(topicObj, this, null);

    topicObj = myCtx.allocTopic("MyTopic");;
    LBMSource mySrc = myCtx.createSource(topicObj);

    try { Thread.sleep(1000); } catch (Exception e) {}

    ByteBuffer mySrcBuffer = ByteBuffer.allocateDirect(128);
    mySrcBuffer.position(0); mySrcBuffer.put("123456789".getBytes());
    for (i = 0; i < 5; i++) {
      mySrc.send(mySrcBuffer, 0, 9, LBM.MSG_FLUSH, null);
      try { Thread.sleep(1000); } catch (Exception e) {}
    }

    try { Thread.sleep(1000); } catch (Exception e) {}

    myRcv.close();
    mySrc.close();
    System.out.println("myStatsThread.terminate");
    myStatsThread.terminate();
    myCtx.close();
  }  // run

  public int onReceive(Object cbArg, LBMMessage msg) {
    msg.dispose();
    return 0;
  }  // onReceive

}  // class MonSelf


class StatsThread implements Runnable {
  private int statsIntervalSec;
  private LBMContext ctx;
  private Thread thisThread;
  private boolean running;

  public StatsThread(LBMContext ctx, int statsIntervalSec) {
    this.statsIntervalSec = statsIntervalSec;
    this.ctx = ctx;
    thisThread = null;
    running = false;
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
      } catch (Exception e) {}
    }
  }  // terminate

  public void run() {
    int secsSinceLastPrint = statsIntervalSec;  // Print stats immediately on start.
    while (running) {
      if (secsSinceLastPrint >= statsIntervalSec) {
        printStats(ctx);
        secsSinceLastPrint = 0;
      }
      try { Thread.sleep(1000); } catch (Exception e) {}
      secsSinceLastPrint++;
    }  // while running

    // Final stats.
    printStats(ctx);
  }  // run

  public static void printStats(LBMContext ctx) {
    System.out.println("printStats");
  }  // printStats
} // StatsThread
