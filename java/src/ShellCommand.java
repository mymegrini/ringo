public interface ShellCommand {

  public String getCommand();

  public int executeCommand(String args[]);
}
