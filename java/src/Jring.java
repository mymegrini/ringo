import java.util.*;
import java.net.*;
import java.io.*;
import java.math.*;

public class Jring
{
  ////////////////////////////////////////////////////////////////////////////////
  // Internal classes
  ////////////////////////////////////////////////////////////////////////////////

  public class Verbose {

    public Verbose() {}

    public void print(String message) {
      System.out.print(message);
    }

    public void println(String message) {
      print(message + "\n");
    }
  }
  ////////////////////////////////////////////////////////////////////////////////
  // Attributes
  ////////////////////////////////////////////////////////////////////////////////

  public Verbose verbose;
  public Entity ent;

  public Thread shell;
  public MessageManager messageManager;
  public RingTester ringTester;
  ////////////////////////////////////////////////////////////////////////////////
  // Constructors
  ////////////////////////////////////////////////////////////////////////////////

  public Jring() {
    verbose = new Verbose();
    ent     = null;
  }

  public Jring(Entity _ent) {
    verbose = new Verbose();
    ent     = _ent;
  }
  ////////////////////////////////////////////////////////////////////////////////
  // Methods
  ////////////////////////////////////////////////////////////////////////////////

  public static boolean isIp(String ip) {
    try {
      InetAddress.getByName(ip);
      return true;
    } catch (Exception e) {
      return false;
    }
  }


  public static boolean isProtocolIp(String ip) {
    return ip.length() == 15 && isIp(ip);
  }


  public static String ipProtocolFormat(String ip) {
    String s[] = ip.split("\\.");
    return leftPadString(s[0], 3, '0') + "." +
      leftPadString(s[1], 3, '0') + "." +
      leftPadString(s[2], 3, '0') + "." +
      leftPadString(s[3], 3, '0');
  }

  /** 
   * Pad a String witch c character to the size, 
   * return a substring if size is less
   * than s length.
   */
  public static String rightPadString(String s, int size, char c) {
    if (s.length() > size)
      return s.substring(0, size);
    for (int i = s.length(); i < size; i++)
      s += c;
    return s;
  }


  /** 
   * Pad a String witch c character to the size, 
   * return a substring if size is less
   * than s length.
   */
  public static String leftPadString(String s, int size, char c) {
    String pad = "";
    if (s.length() > size)
      return s.substring(s.length()-size-1, s.length());
    for (int i = s.length(); i < size; i++)
      pad += c;
    return pad + s;
  }


  /** 
   * Pad a String witch space character to the size, 
   * return a substring if size is less
   * than s length.
   */
  public static String padString(String s, int size) {
    return rightPadString(s, size, ' ');
  }


  public static String truncateString(String s, int size) {
    if (s.length() > size)
      s = s.substring(0, size);
    return s;
  }


  public static int readInt(Scanner sc, String message) {
    int i;
    System.out.print(message);
    String input;
    while (true) {
      input = sc.nextLine();
      try {
        i = Integer.parseInt(input);
        return i;
      } catch (Exception e) {
        System.out.println("Integer value needed.");
      }
    }
  }


  public static int readInt(Scanner sc, String message, int defaultValue) {
    int i;
    System.out.print(message + " default " + defaultValue + ": ");
    String input;
    input = sc.nextLine();
    try {
        i = Integer.parseInt(input);
        return i;
    } catch (Exception e) {
      return defaultValue;
    }
  }


  public static void main(String[] args){
    // Entity initialization
    final int CREATE       = 0,
          JOIN             = 1,
          DUPLICATE        = 2;
    final String modeStr[] = { "creation", "insertion", "duplication" };
    Scanner sc = new Scanner(System.in);
    String name = "", input;
    int udp     = -1,
        tcp     = -1,
        mode    = -1;

    if (args.length == 0) {
      System.out.print("Choose a name: ");
      input = sc.nextLine();
      name  = truncateString(input,8);
      udp   = readInt(sc, "Enter your udp port: ");
      tcp   = readInt(sc, "Enter your tcp port: ");
      mode  = readInt(sc, "Choose a mode (1:create/2:join/3:duplicate): ") - 1;
      System.out.println("MODE: " + mode);
    }
    // name, udp, tcp, mode
    else if (args.length == 4) {
      name = truncateString(args[0],8);
      try {
        udp  = Integer.parseInt(args[1]);
        tcp  = Integer.parseInt(args[2]);
        mode = Integer.parseInt(args[3]) - 1;
      } catch (Exception e) {
        System.out.println("Integer value needed.");
        System.exit(0);
      }
    }
    else {
      System.out.println("Usage:\t<name> <udp> <tcp> <mode>");
      System.exit(0);
    }

    Jring jring = new Jring();
    try {
      jring.ent   = new Entity(jring, name, udp, tcp);
    } catch (Exception e) {
      System.out.println("Network exception during initialisation.");
      System.out.println(e);
      e.printStackTrace();
      System.exit(0);
    }

    String mdiffAddr, host;
    int mdiffPort, port;
    boolean success = false;

    // Executing the mode
    try {
      switch (mode) {
        case CREATE:
          System.out.println("Enter network address, default 225.0.0.1: ");
          input = sc.nextLine();
          if (input.length() == 0)
            mdiffAddr = "225.0.0.1";
          else
            mdiffAddr = ipProtocolFormat(input);
          mdiffPort = readInt(sc, "Enter network port", 6667);
          success = jring.createRing(mdiffAddr, mdiffPort);
          break;
        case JOIN:
          System.out.println("Enter hostname: ");
          host = sc.nextLine();
          port = readInt(sc, "Enter port: ");
          success = jring.insert(host, port);
          break;
        case DUPLICATE:
          System.out.println("Enter network address, default 225.0.0.1: ");
          input = sc.nextLine();
          if (input.length() == 0)
            mdiffAddr = "227.0.0.1";
          else
            mdiffAddr = ipProtocolFormat(input);
          mdiffPort = readInt(sc, "Enter network port", 6667);
          System.out.println("Enter hostname: ");
          host = sc.nextLine();
          port = readInt(sc, "Enter port: ");
          success = jring.duplicate(host, port, mdiffAddr, mdiffPort);
          break;
      }
    }
    catch (Exception e) {
      System.out.println("Exception during " + modeStr[mode] + ".");
      System.out.println(e);
      e.printStackTrace();
      System.exit(0);
    }
    if (!success) {
      System.out.println("Sorry, failed " + modeStr[mode] + ".");
      System.exit(0);
    }

    // Threads initialization
    try {
      jring.messageManager = new MessageManager(jring);
      jring.messageManager.startMessageAdmin();
    } catch (Exception e) {
      System.out.println("Error initializing message manager.");
      System.exit(0);
    }

    jring.ringTester = new RingTester(jring);
    Thread ringTesterT = new Thread(jring.ringTester);
    ringTesterT.start();

    jring.shell = new Thread(new Shell(jring, name));
    jring.shell.start();
    try {
      jring.shell.join();
    } catch (Exception e) {
      System.out.println("Shell interrupded.");
    }
  }


  public boolean createRing(String mdiffaddr, int mdiffport) throws Exception {
    InetSocketAddress next  = new InetSocketAddress(InetAddress.getLocalHost(), ent.udp);
    InetSocketAddress mdiff = new InetSocketAddress(InetAddress.getByName(mdiffaddr), mdiffport);
    System.out.println("PORT; " + mdiff.getPort());

    ent.addNextEntity(next, mdiff);
    System.out.println("Ring created.");
    return true;
  }


  public boolean insert(String adress,int tcp) throws Exception {
    Socket socket = new Socket(InetAddress.getByName(adress),tcp);
    BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
    String mess=br.readLine();
    Entity.NextEntity next = parseWelc(mess);
    boolean success = false;
    if (next != null) {
      mess= "NEWC "+ent.ip+" "+Entity.add_zero(ent.udp,4);
      pw.println(mess);
      pw.flush();
      mess=br.readLine();
      if (mess == null) {
        mess=br.readLine();
        success = false;
        System.out.println("NULL " + mess);
      } else
      if(mess.equals("ACKC")) {
        ent.insertNextEntity(next);
        success = true;
      }
    }
    if (success)
      System.out.println("Insertion successful.");
    else
      System.out.println("Server bad response: \"" + mess + "\".");
    br.close();
    pw.close();
    socket.close();
    return success;
  }


  public boolean duplicate(String adress,int tcp, String mdiff, int portmdiff) throws Exception {
    Socket socket = new Socket(adress,tcp);
    BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
    String mess=br.readLine();
    Entity.NextEntity next = parseWelc(mess);
    boolean success = false;
    if (next != null) {
      mess= "DUPL "+ent.ip+" "+ String.valueOf(tcp) + " " + mdiff + " " + String.valueOf(portmdiff);
      pw.println(mess);
      pw.flush();
      mess=br.readLine();
      if(mess.substring(0, 5).equals("ACKC ") && mess.length() == 9) {
        try {
          int portnext = Integer.parseInt(mess.substring(5, mess.length()-1));
          next.entity = new InetSocketAddress(next.entity.getAddress(), portnext);
          ent.addNextEntity(next);
          success = true;
        } catch (Exception e) {
          success = false;
        }
      }
    }
    if (success)
      System.out.println("Insertion successful.");
    else
      System.out.println("Server bad response: \"" + mess + "\".");
    br.close();
    pw.close();
    socket.close();
    return success;
  }


  public static Entity.NextEntity parseWelc(String mess) {
    if (mess.charAt(4) != ' ' || mess.charAt(20) != ' ' || mess.charAt(25) != ' ' ||
        mess.charAt(41) != ' ' || mess.length() != 46)
      return null;
    String[] split = mess.substring(0, mess.length()-1).split(" ");
    if (split.length != 5)
      return null;
    // ip size
    if (split[1].length() != 15 || split[3].length() != 15)
      return null;
    try {
      InetAddress addr = InetAddress.getByName(split[1]);
      InetAddress addrdiff = InetAddress.getByName(split[3]);
      int port = Integer.parseInt(split[2]);
      int portdiff = Integer.parseInt(split[4]);
      return new Entity.NextEntity(addr, port, addrdiff, portdiff);
    } catch (Exception e) {
      return null;
    }


  }


  public static void cmd_ls(){
    try{
      Process process = Runtime.getRuntime().exec("ls -a");
      BufferedReader stdout = new BufferedReader(new InputStreamReader( process.getInputStream()));
      String line = stdout.readLine() ;
      while(line != null){
        System.out.println(line);
        line = stdout.readLine() ;
      }
      stdout.close(); 
    }
    catch (Exception e) {
      e.printStackTrace();
      System.exit(-1);
    }
  }
}
