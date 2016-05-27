/** RdiffCommand .java
 */ 

public class RdiffCommand implements ShellCommand, MessageApplicationAction
{
  Jring jring;

  public RdiffCommand(Jring _jring) { 
    jring = _jring;
  }

  public String getCommand() { 
    return "rdiff";
  }

  public int executeCommand(String args[]) { 
      if (args.length > 1) {
        String mess = args[1];
        for (int i = 2; i < args.length; i++)
          mess += args[i];
        mess = Jring.truncateString(mess, 489);
        jring.messageManager.sendAppMessage(getApplicationId(), mess);
        jring.verbose.println("Diff message: \"" + mess + "\" sent.");
      }
      return 0;
  }


  public String getApplicationId() {
    return "DIFF####";
  }


  public int executeApplicationAction(String message, String content, boolean lookupFlag) {
    if (!lookupFlag) {
      jring.verbose.println("DIFF####: " + content);
      jring.ent.sendPacketAll(message);
    }
    else {
      jring.verbose.println("Message \"" + message + "\" already seenn, nothing to do.");
    }
    return 0;
  }
}
//RdiffCommand .java 

