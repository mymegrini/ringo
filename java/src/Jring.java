import java.util.*;
import java.net.*;
import java.io.*;
import java.math.*;

public class Jring{
    static Entity ent;
    static ArrayList<String> mess_list;
    static  ArrayList<String> mess_list_dupl;
    
    public static void main(String[] args){
        mess_list = new ArrayList<String>();
        mess_list_dupl = new ArrayList<String>();
        InetAddress host_ip =host_ip();
        System.out.println("Your ip adress : "+host_ip.toString().substring(1));
        Scanner sc = new Scanner(System.in);
        String id="";
        int u=0,t=0,d=0;
        ServerSocket server=null;
        DatagramSocket dso_udp=null;
        if(args.length!=0 && args[args.length-1].substring(1).equals("auto")){
            if(args[args.length-1].charAt(0)=='1'){
                id="Soheib";
                u=8888;
                t=8887;
                d=8886;
            }
            if(args[args.length-1].charAt(0)=='2'){
                id="Nicolas";
                u=8889;
                t=8880;
                d=8881;
            }
            if(args[args.length-1].charAt(0)=='3'){
                id="Younes";
                u=8885;
                t=8884;
                d=8883;
            }
            try{
                server = new ServerSocket(t);
                dso_udp = new DatagramSocket(u);
            }
            catch(Exception e){
                System.out.println(e);
                e.printStackTrace();
            }
        }
        else{
            System.out.println("Give your id");
            id = sc.nextLine();
            while(id.length()>8){
                System.out.println("The maximum length of your id is 8 characters \n Give a new id");
                id = sc.nextLine();
            }
            System.out.println("Give your udp port");
            u = sc.nextInt();
            while(u<1024 || u>9999){
                System.out.println("The udp port is not correct, give one between 1024 and 9999");
                u = sc.nextInt();
            }
            boolean dso_use=true;
            while(dso_use){
                try{
                    dso_udp = new DatagramSocket(u);
                    dso_use=false;
                }
                catch(BindException e){
                    System.out.println("The udp port is already used, give an other one between 1024 and 9999");
                    u= sc.nextInt();
                }
                catch(Exception e){
                    System.out.println(e);
                    e.printStackTrace();
                }
            }
            System.out.println("Give your tcp port");
            t = sc.nextInt();
            while(t<1024 || t>9999){
                System.out.println("The tcp port is not correct, give one between 1024 and 9999");
                t = sc.nextInt();
            }
            boolean serv_use=true;
            while(serv_use){
                try{
                    server = new ServerSocket(t);
                    serv_use=false;
                }
                catch(BindException e){
                    System.out.println("The tcp port is already used, give an other one between 1024 and 9999");
                    t = sc.nextInt();
                }
                catch(Exception e){
                    System.out.println(e);
                    e.printStackTrace();
                }
            }
            if(args.length==1 || args.length==1){
                System.out.println("Give the diffusion port");
                d = sc.nextInt();
                while(t<1024 || t>9999){
                    System.out.println("The diffusion port is not correct, give one between 1024 and 9999");
                    d = sc.nextInt();
                }
            }
        }
        ent = new Entity(id,host_ip.toString().substring(1),u,t,"225.0.0.1",d);
        boolean stage1=true;
        if(args.length==0 || args.length==1){
            ent.ip_next=ent.ip;
            ent.port_next=u;
        }
        else{   
            if(args.length==3 || args.length==4){
                if(args[0].equals("insert")) stage1=insert(args[1],Integer.parseInt(args[2]));
                if(args[0].equals("duplicate")) stage1=duplicate(args[1],Integer.parseInt(args[2]));
            }
            else stage1=false;
        }
        if(stage1){
            try{
                //Diff
                Diff_thread diff_mode = new Diff_thread(ent,ent.mdiff_ip,ent.mdiff_port);
                Thread diff_t = new Thread(diff_mode);
                diff_t.start();
                //Tcp
                Tcp_thread tcp_mode = new Tcp_thread(ent,server);
                Thread tcp_t = new Thread(tcp_mode);
                tcp_t.start();
                //Udp
                Udp_thread udp_mode = new Udp_thread(ent,mess_list,mess_list_dupl,dso_udp,diff_mode.mso);
                Thread udp_t = new Thread(udp_mode);
                udp_t.start();
                diff_mode.dso_udp=dso_udp;
                diff_mode.server_tcp=server;
                //Send a message
                byte[] data = new byte[512];
                DatagramSocket dso = new DatagramSocket();
                DatagramPacket packet_send;
                String mess_send;
                Down_thread dwn;
                Thread dwn_t;
                String m_id;
                int size_mess;
                boolean mess_recognize=false;
                while(true){
                    mess_recognize=false;
                    mess_send= sc.nextLine();
                    m_id=message_id();
                    if(mess_send.equals("help")){
                        cmd_help();
                    }
                    if(mess_send.equals("ls")){
                        cmd_ls();
                    }
                    if(mess_send.equals("info")){
                        ent.all_info();
                    }
                    if(mess_send.equals("whos")){
                        mess_send="WHOS "+m_id;
                        mess_recognize=true;
                    }
                    if(mess_send.equals("gbye")){
                        mess_recognize=true;
                        tcp_mode.server.close();
                        mess_send="GBYE "+m_id+" "+ent.ip+" "+Entity.add_zero(ent.udp,4)+" "+ent.ip_next+" "+Entity.add_zero(ent.port_next,4);
                    }
                    if(mess_send.equals("test")){
                        mess_send="TEST "+m_id+" "+ent.mdiff_ip+" "+Entity.add_zero(ent.mdiff_port,4);
                        mess_list.add(m_id);
                        Udp_thread.send_mess(ent,dso,mess_send);
                        dwn = new Down_thread(ent,mess_list,m_id,0,ent.mdiff_ip,ent.mdiff_port);
                        dwn_t = new Thread(dwn);
                        dwn_t.start();
                        if(ent.port_next2!=-1){
                            m_id=message_id();
                            mess_send="TEST "+m_id+" "+ent.mdiff_ip2+" "+Entity.add_zero(ent.mdiff_port2,4);
                            mess_list.add(m_id);
                            Udp_thread.send_mess(ent,dso,mess_send);
                            dwn = new Down_thread(ent,mess_list,m_id,0,ent.mdiff_ip2,ent.mdiff_port2);
                            dwn_t = new Thread(dwn);
                            dwn_t.start();
                        }
                    }
                     if(mess_send.equals("testplus")){
                        mess_send="TEST "+m_id+" "+ent.mdiff_ip+" "+Entity.add_zero(ent.mdiff_port,4);
                        System.out.println("How many times do you like to check the ring if it's not safe ?");
                        int times = sc.nextInt();
                        mess_list.add(m_id);
                        Udp_thread.send_mess(ent,dso,mess_send);
                        dwn = new Down_thread(ent,mess_list,m_id,0,ent.mdiff_ip,ent.mdiff_port);
                        dwn_t = new Thread(dwn);
                        dwn_t.start();
                        if(ent.port_next2!=-1){
                            m_id=message_id();
                            mess_send="TEST "+m_id+" "+ent.mdiff_ip2+" "+Entity.add_zero(ent.mdiff_port2,4);
                            mess_list.add(m_id);
                            Udp_thread.send_mess(ent,dso,mess_send);
                            dwn = new Down_thread(ent,mess_list,m_id,0,ent.mdiff_ip2,ent.mdiff_port2);
                            dwn_t = new Thread(dwn);
                            dwn_t.start();
                        }
                    }
                    if(mess_send.equals("diff")){
                        mess_recognize=true;
                        System.out.println("Give your message (less than 512-x characters)");
                        mess_send= sc.nextLine();
                        size_mess=mess_send.length();
                        if(size_mess<486) mess_send="APPL "+m_id+" "+"DIFF#### "+Entity.add_zero(size_mess,3)+" "+mess_send;
                        else{
                            System.out.println("The length of message is more than 482o");
                            mess_recognize=false;
                        }
                    }
                    if(mess_send.equals("trans")){
                        mess_recognize=true;
                        System.out.println("Give the name of the file");
                        mess_send= sc.nextLine();
                        size_mess=mess_send.length();
                        if(size_mess<482) mess_send="APPL "+m_id+" "+"TRANS### "+"REQ "+Entity.add_zero(size_mess,2)+" "+mess_send;
                        else{
                            System.out.println("The length of message is more than 482o");
                            mess_recognize=false;
                        }
                    }
                    if(mess_recognize){
                        mess_list.add(m_id);
                        Udp_thread.send_mess(ent,dso,mess_send);
                    }
                }
            }
            catch(Exception e){
                System.out.println(e);
                e.printStackTrace();
            }
        }
    }

    public void println(String message) {
      print(message + "\n");
    }
  }
  ////////////////////////////////////////////////////////////////////////////////
  // Attributes
  ////////////////////////////////////////////////////////////////////////////////

    public static boolean duplicate(String adress,int tcp){
        try{
            Scanner sc = new Scanner(System.in);
            /*System.out.println("Give the ip adress of the new ring");
              String ip_mdiff2 = sc.nextLine();*/
            String ip_mdiff2 = "225.0.0.1";
            System.out.println("Give the diffusion port of the new ring");
            int port_mdiff2=sc.nextInt();
            Socket socket = new Socket(adress,tcp);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            String mess_recv=br.readLine();
            Welc_mess data_welc = Welc_mess.parse_welc(mess_recv);
            boolean dupl=false;
            if(data_welc!=null){
                String mess_send = "DUPL "+Entity.ip_form(ent.ip)+" "+Entity.add_zero(ent.udp,4)+" "+Entity.ip_form(ip_mdiff2)+" "+Entity.add_zero(port_mdiff2,4);
                pw.println(mess_send);
                pw.flush();
                mess_recv=br.readLine();
                String[] tab = mess_recv.split(" ");
                try{
                    if(tab.length==2){
                        ent.ip_next=Entity.ip_form(adress);
                        ent.port_next=Integer.parseInt(tab[1]);
                        ent.mdiff_ip=Entity.ip_form(ip_mdiff2);
                        ent.mdiff_port=port_mdiff2;
                        dupl=true;
                        System.out.println("duplicate : Successful Connexion !");
                    }
                }catch(Exception e){
                    System.out.println("Wrong structure "+mess_recv);
                }
            }
            else System.out.println("duplicate : Connexion Failed !");
            br.close();
            pw.close();
            socket.close();
            return dupl;
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
            return false;
        }
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

    public static void cmd_help(){
        System.out.println("info : Display informations on current entity.");
        System.out.println("whos : Getting to know each other...");
        System.out.println("gbye : Exit.");                        
        System.out.println("diff : Send messages on the ring.");
        System.out.println("test : Know if the ring is still correct.");
        System.out.println("testplus : Know if the ring is still correct with verification");
        System.out.println("trans : Have a file that another computer in the ring have.");
        System.out.println("ls : Know what is in your repository.");
        System.out.println("insert duplication ?");
    }
}
