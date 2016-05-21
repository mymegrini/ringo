/** RingTester .java
 */ 
import java.util.*;

public class RingTester implements Runnable
{
  ////////////////////////////////////////////////////////////////////////////////
  // Internal classes
  ////////////////////////////////////////////////////////////////////////////////

  private class Tester implements Runnable {
    public void run() {
      // non blocking listening of all the socks
    }
  }

  public Jring jring;

  public RingTester (Jring _jring){
    jring = _jring;
  }


  public void run() {
    // restart the tester whenever a ring is added or deleted
  }
}
//RingTester .java 

