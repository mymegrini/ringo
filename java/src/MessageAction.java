/** MessageAction .java
 */ 

public interface MessageAction
{
  public int executeAction(String message, String content, boolean lookupFlag);

  public String getAction();
}
//MessageAction .java 
