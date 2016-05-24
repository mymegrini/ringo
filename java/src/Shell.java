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
    jring = _jring;
    prompt = "[" + promptInfo + "] > ";

    // Add commands
    command = new ArrayList<>();
    command.add(exit);
    command.add(whos);
    command.add(new RdiffCommand(jring));
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
        cmd.executeCommand(args);
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
      System.out.println("Command " + args[0] + " not found.");
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  private ShellCommand exit = new ShellCommand() {
    public String getCommand() { return "exit"; }
    public int executeCommand(String[] args) { 
      // System.out.println("Running exit...");
      // jring.shell.interrupt(); 
      // System.out.println("Exit run.");
      System.exit(0);
      return 0;
    }
  };

  private ShellCommand whos = new ShellCommand() {
    public String getCommand() { return "whos"; }
    public int executeCommand(String[] args) { 
      jring.messageManager.sendMessage("WHOS", "");
      return 0;
    }
  };

}
//Shell .java 

