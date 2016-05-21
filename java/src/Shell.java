/** Shell .java
 */ 

import java.util.*;
import java.io.*;

public class Shell implements Runnable
{
  private ArrayList<ShellCommand> command;
  private String prompt;
  private Jring jring;

  public Shell (Jring _jring, String promptInfo){
    command = new ArrayList<>();
    jring = _jring;
    prompt = "[" + promptInfo + "] > ";

    // Add commands
    command.add(exit);
  }


  public void run() {
    Scanner sc = new Scanner(System.in);
    while (true) {
      System.out.print(prompt);
      String commandLine = sc.nextLine();
      parseAndExecute(commandLine);
    }
  }

  private void parseAndExecute(String commandLine) {
    if (commandLine.length() == 0)
      return;
    String[] args = commandLine.split(" ");
    for (ShellCommand cmd: command) {
      if (args[0].equals(cmd.getCommand())) {
        cmd.execute(args);
        return;
      }
    }
    executeExternalCommand(args);
  }


  public static void executeExternalCommand(String args[]) {
    try {
      Process p = Runtime.getRuntime().exec(args);
      BufferedReader output = new BufferedReader(new InputStreamReader(p.getInputStream()));
      BufferedReader error = new BufferedReader(new InputStreamReader(p.getInputStream()));
      String line = "";

      while ((line = output.readLine()) != null) {
        System.out.println(line);
      }

      while ((line = error.readLine()) != null) {
        System.out.println(line);
      }

      p.waitFor();
    } catch (IOException e) {
      e.printStackTrace();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  public ShellCommand exit = new ShellCommand() {
    public String getCommand() { return "exit"; }
    public int execute(String[] args) { 
      // System.out.println("Running exit...");
      // jring.shell.interrupt(); 
      // System.out.println("Exit run.");
      System.exit(0);
      return 0; }
  };
}
//Shell .java 

