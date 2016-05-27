/** MessageApplicationAction .java
 */ 

public interface MessageApplicationAction
{
  public String getApplicationId();

  public int executeApplicationAction(String message, String appContent, boolean lookupFlag);
}
//MessageApplicationAction .java 

