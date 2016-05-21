/** MessageAction .java
 */ 

public interface MessageAction
{
  public abstract int execute(String message, String content, boolean lookupFlag);

  public abstract String getType();
}
//MessageAction .java 
