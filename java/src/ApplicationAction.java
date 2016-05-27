/** ApplicationAction .java
*/ 
import java.util.*;

public class ApplicationAction implements MessageAction
{
  ////////////////////////////////////////////////////////////////////////////////
  // Attributes
  ////////////////////////////////////////////////////////////////////////////////

  private ArrayList<MessageApplicationAction> appAction;
  private Jring jring;
  ////////////////////////////////////////////////////////////////////////////////
  // Constructors
  ////////////////////////////////////////////////////////////////////////////////

  public ApplicationAction(Jring _jring) {
    jring = _jring;
    appAction = new ArrayList<>();
    appAction.add(new RdiffCommand(jring));
    // appAction = new ArrayList<>() {{
    //   add(new RdiffCommand(jring));
    // }};
  }
  ////////////////////////////////////////////////////////////////////////////////
  // Methods
  ////////////////////////////////////////////////////////////////////////////////

  public String getAction() { return "APPL"; }

  public int executeAction(String message, String content, boolean lookupFlag) {
    if (content.charAt(8) != ' ') {
      jring.verbose.println("Application content not following the protocol: \"" + content + "\"");
      return -1;
    }
    String type = content.substring(0,8);
    for (MessageApplicationAction action : appAction) 
      if (type.equals(action.getApplicationId())) {
        return action.executeApplicationAction(message, content.substring(9, content.length()), lookupFlag);
      }
    return 0;
  }
}


//ApplicationAction .java 

