public interface ShellCommand {

  public String getCommand();

  public abstract int execute(String args[]);
}
