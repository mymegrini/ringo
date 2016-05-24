/** TcpServer .java
 */ 
import java.net.*;
import java.io.*;

public class TcpServer implements Runnable
{
  ////////////////////////////////////////////////////////////////////////////////
  // Internal classes
  ////////////////////////////////////////////////////////////////////////////////

  private class Newc {
    public final String ip;
    public final int port;

    private Newc(String _ip, int _port) {
      ip   = _ip;
      port = _port;
    }

    public Newc parseNewc(String mess) {
      if (mess.charAt(4) != ' ' || mess.charAt(20) != ' ' || 
          mess.charAt(25) != '\n' || mess.length() != 25) {
        ent.jring.verbose.println("Message newc " + mess +
            " not following the protocol.");
        return null;
          }
      String[] split = mess.substring(0, mess.length()-1).split(" ");
      if (!split[0].equals("NEWC") || split.length != 3) {
        ent.jring.verbose.println("Message newc " + mess +
            " not following the protocol.");
        return null;
      }
      int port;
      if (! ent.jring.isProtocolIp(split[1])) {
        ent.jring.verbose.println("Message newc " + mess +
            " not following the protocol.");
        return null;
      }
      try {
        port = Integer.parseInt(split[2]);
      } catch (Exception e) {
        ent.jring.verbose.println("Message newc " + mess +
            " not following the protocol.");
        return null;
      }
      return new Newc(split[1], port);
    }
  }
  private final Entity ent;

  public TcpServer (Entity _ent){
    ent = _ent;
  }

  public void run() {
    try {
      ServerSocket sock = new ServerSocket(ent.tcp);
      while (true) {
        Socket client = sock.accept();
      }
    } catch (Exception e) {
      System.out.println("Error during tcp server creation.");
      System.exit(0);
    }
  }


  private void manageClient(Socket socket) throws Exception {
    BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
    String mess ="WELC "+ent.ip_next+" "+Entity.add_zero(ent.port_next,4)+" "+ent.mdiff_ip+" "+Entity.add_zero(ent.mdiff_port,4);
    pw.println(mess);
    pw.flush();
    mess=br.readLine();
    String messType = messageType(mess);
    switch (messType) {
      case "NEWC":
        manageInsertion(pw, mess);
        break;
      case "DUPL":
        manageDuplication(pw, br, mess);
        break;
      default:
        ent.jring.verbose.println("TCP incoming message \"" + mess +
            "\" not following the protocol.");
        return;
    }
    br.close();
    pw.close();
    socket.close();
  }


  private void manageInsertion(PrintWriter pw, String mess) {
    InetSocketAddress nextAddr = parseNewc(mess);
    if (nextAddr == null) {
      ent.jring.verbose.println("TCP newc \"" + mess +
          "\" not following the protocol.");
      return;
    }
    ent.insertNextEntity(nextAddr);
    pw.println("ACKC");
    pw.flush();
  }


  private void manageDuplication(PrintWriter pw, BufferedReader br, String mess) {
    Entity.NextEntity next = parseDupl(mess);
    if (next == null) {
      ent.jring.verbose.println("TCP dupl \"" + mess +
          "\" not following the protocol.");
      return;
    }

  }
  ////////////////////////////////////////////////////////////////////////////////
  // Static
  ////////////////////////////////////////////////////////////////////////////////

  
  private static String messageType(String mess) {
    if (mess.charAt(4) != ' ')
      return null;
    return mess.substring(0, 4);
  }


  private static InetSocketAddress parseNewc(String mess) {
      if (mess.charAt(20) != ' ' || mess.charAt(25) != '\n' || mess.length() != 25)
        return null;
      String[] split = mess.substring(0, mess.length()-1).split(" ");
      if (split.length != 3)
        return null;
      // ip size
      if (split[1].length() != 15)
        return null;
      try {
        InetAddress addr = InetAddress.getByName(split[1]);
        int port = Integer.parseInt(split[2]);
        return new InetSocketAddress(addr, port);
      } catch (Exception e) {
        return null;
      }
  }

  private static Entity.NextEntity parseDupl(String mess) {
      if (mess.charAt(20) != ' ' || mess.charAt(25) != ' ' ||
          mess.charAt(41) != ' ' || mess.charAt(46) != '\n' || mess.length() != 46)
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
}
//TcpServer .java 

