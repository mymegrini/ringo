/** RingTester .java
 */ 
import java.util.*;

public class RingTester implements Runnable
{
  ////////////////////////////////////////////////////////////////////////////////
  // Internal classes
  ////////////////////////////////////////////////////////////////////////////////

  public class ActionTest implements MessageAction {
    public String getType() { return "TEST"; }
    public int execute(String message, String content, boolean lookupFlag) {
      try {
        if (content.charAt(15) == ' ') {
          int port = Integer.parseInt(content.substring(16, content.length()));
          int ring = jring.ent.findRing(content.substring(0, 15), port);
          if (ring != -1) {
            if (lookupFlag) { // message already seen, meaning sent by us
              notifyWorking(ring);
            } else {
              jring.ent.sendPacketAll(message);
            }
            return 0;
          }
        }
        jring.verbose.println("Test content not following the protocol: " + message + "\"");
        return -1;
      } catch (Exception e) {
        return -1;
      }
    }
  }
  ////////////////////////////////////////////////////////////////////////////////
  // Attributes
  ////////////////////////////////////////////////////////////////////////////////

  private Jring jring;
  private ArrayList<Boolean> checked;
  private int nchecked;

  ////////////////////////////////////////////////////////////////////////////////
  // Constructor
  ////////////////////////////////////////////////////////////////////////////////

  public RingTester (Jring _jring){
    jring = _jring;
  }
  ////////////////////////////////////////////////////////////////////////////////
  // Methods
  ////////////////////////////////////////////////////////////////////////////////

  public void run() {
    Entity ent = jring.ent;
    while (true) {
      try {
        Thread.sleep(10000);
      } catch (Exception e) {
        jring.verbose.println("Ring tester interrupted while sleeping. He said " +
            "look, I'm tired now!");
      }
      nchecked = 0;
      checked = new ArrayList<>(ent.ringNumber());
      ent.readLock();
      for (Entity.NextEntity next : ent.getNextEntity()) {
        checked.add(false);
        jring.messageManager.sendMessage(next, "TEST", next.mdiffIp + " " + 
            next.mdiff.getPort());
      }
      ent.readUnlock();
      try {
        wait(10000);
      } catch (Exception e) {
      }
      boolean allClean = true;
      if (nchecked < ent.ringNumber()) {
        ent.writeLock();
        for (int i = 0; i < checked.size(); i++)
          if (checked.get(i)) {
            jring.verbose.println("Ring " + i + " broken.");
            ent.getNextEntity().remove(i);
            allClean = false;
          }
        ent.writeUnlock();
        if (allClean) {
          jring.verbose.println("All ring checked.");
        }
      }
    }
  }

  public synchronized void notifyWorking(int i) {
    checked.set(i, false);
    if (++nchecked == jring.ent.ringNumber())
      notify();
  }
}
//RingTester .java 

