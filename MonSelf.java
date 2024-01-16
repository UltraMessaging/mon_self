/* MonSelf.java -  Example demonstration of "StatsThread" class.
 * See https://github.com/UltraMessaging/cfg_dump */
/*
  (c) Copyright 2023-2024 Informatica Corporation
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

    LBM.setConfiguration("tst.cfg");

    LBMContext myCtx1 = new LBMContext();
    LBMContext myCtx2 = new LBMContext();

    StatsThread myStatsThread1 = new StatsThread(myCtx1, "ctx1", 2);
    myStatsThread1.start();
    try { Thread.sleep(1000); } catch (Exception e) {}
    StatsThread myStatsThread2 = new StatsThread(myCtx2, "ctx2", 2);
    myStatsThread2.start();

    LBMTopic topicObj = myCtx1.lookupTopic("MyTopic");
    LBMReceiver myRcv = myCtx1.createReceiver(topicObj, this, null);

    topicObj = myCtx1.allocTopic("MyTopic");
    LBMSource mySrc1 = myCtx1.createSource(topicObj);

    topicObj = myCtx2.allocTopic("MyTopic");
    LBMSource mySrc2 = myCtx2.createSource(topicObj);

    try { Thread.sleep(500); } catch (Exception e) {}

    ByteBuffer mySrcBuffer = ByteBuffer.allocateDirect(128);
    mySrcBuffer.position(0); mySrcBuffer.put("123456789".getBytes());
    for (i = 0; i < 5; i++) {
      mySrc1.send(mySrcBuffer, 0, 9, LBM.MSG_FLUSH, null);
      mySrc2.send(mySrcBuffer, 0, 9, LBM.MSG_FLUSH, null);
      try { Thread.sleep(1000); } catch (Exception e) {}
    }

    try { Thread.sleep(1000); } catch (Exception e) {}

    myRcv.close();
    mySrc1.close();
    mySrc2.close();
    System.out.println("terminate stats thread1");
    myStatsThread1.terminate();
    System.out.println("terminate stats thread2");
    myStatsThread2.terminate();
    myCtx1.close();
    myCtx2.close();
  }  // run

  public int onReceive(Object cbArg, LBMMessage msg) {
    msg.dispose();
    return 0;
  }  // onReceive

}  // class MonSelf
