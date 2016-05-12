import java.util.*;
import java.net.*;
import java.io.*;

public class Jring{
    static Entity ent;
    static ArrayList<String> mess_list;
    
    public static void main(String[] args){
        mess_list = new ArrayList<String>();
        InetAddress host_ip =host_ip();
        System.out.println(host_ip.toString());
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
                id="Kate";
                u=8889;
                t=8880;
                d=8881;
            }
            if(args[args.length-1].charAt(0)=='3'){
                id="Max";
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
            System.out.println("Give the diffusion port");
            d = sc.nextInt();
            while(t<1024 || t>9999){
                System.out.println("The diffusion port is not correct, give one between 1024 and 9999");
                d = sc.nextInt();
            }
            
        }
        ent = new Entity(id,host_ip.toString().substring(1),u,t,"224.0.0.0",d);
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
                Udp_thread udp_mode = new Udp_thread(ent,mess_list,dso_udp,diff_mode.mso);
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
                    if(mess_send.equals("INFO")){
                        ent.all_info();
                    }
                    //if(mess_send.equals("quit")) break;
                    m_id=message_id();
                    if(mess_send.equals("WHOS")){
                        mess_send="WHOS "+m_id;
                        mess_recognize=true;
                    }
                    if(mess_send.equals("GBYE")){
                        mess_recognize=true;
                        tcp_mode.server.close();
                        mess_send="GBYE "+m_id+" "+ent.ip+" "+Entity.add_zero(ent.udp,4)+" "+ent.ip_next+" "+Entity.add_zero(ent.port_next,4);
                        if(ent.port_next2!=-1){
                            mess_list.add(m_id);
                            Udp_thread.send_mess(ent,dso,mess_send);
                            m_id=message_id();
                            mess_send="GBYE "+m_id+" "+ent.ip+" "+Entity.add_zero(ent.udp,4)+" "+ent.ip_next2+" "+Entity.add_zero(ent.port_next2,4);
                        }
                    }
                    if(mess_send.equals("TEST")){
                        mess_recognize=true;
                        mess_send="TEST "+m_id+" "+ent.mdiff_ip+" "+Entity.add_zero(ent.mdiff_port,4);
                        /*System.out.println("How many times do you like to check the ring if it's not safe ?");
                          int times = sc.nextInt();*/
                        dwn = new Down_thread(ent,mess_list,m_id,0);
                        dwn_t = new Thread(dwn);
                        dwn_t.start();
                    }
                    if(mess_send.equals("DIFF")){
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
                    if(mess_send.equals("TRANS")){
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
    
    public static boolean insert(String adress,int tcp){
        try{
            Socket socket = new Socket(adress,tcp);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            String mess_recv=br.readLine();
            Welc_mess data_welc = Welc_mess.parse_welc(mess_recv);
            boolean ins=false;
            if(data_welc!=null){ 
                ent.ip_next=data_welc.ip_next;
                ent.port_next=data_welc.port_next;
                ent.mdiff_ip=data_welc.mdiff_ip;
                ent.mdiff_port=data_welc.mdiff_port;
                String mess_send = "NEWC "+ent.ip+" "+Entity.add_zero(ent.udp,4);
                pw.println(mess_send);
                pw.flush();
                mess_recv=br.readLine();
                if(mess_recv.equals("ACKC")){
                    ins=true;
                    System.out.println("insert : Successful Connexion !");
                }
            }
            else System.out.println("insert : Connexion Failed !");
            br.close();
            pw.close();
            socket.close();
            return ins;
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
            return false;
        }
    }

    public static boolean duplicate(String adress,int tcp){
        try{
            Scanner sc = new Scanner(System.in);
            /*System.out.println("Give the ip adress of the new ring");
              String ip_mdiff2 = sc.nextLine();*/
            String ip_mdiff2 = "224.0.0.0";
            System.out.println("Give the port of the new ring");
            int port_mdiff2=sc.nextInt();
            Socket socket = new Socket(adress,tcp);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            String mess_recv=br.readLine();
            Welc_mess data_welc = Welc_mess.parse_welc(mess_recv);
            boolean dupl=false;
            if(data_welc!=null){
                String mess_send = "DUPL "+ent.ip+" "+Entity.add_zero(ent.udp,4)+" "+ip_mdiff2+" "+Entity.add_zero(port_mdiff2,4);
                pw.println(mess_send);
                pw.flush();
                mess_recv=br.readLine();
                String[] tab = mess_recv.split(" ");
                try{
                    if(tab.length==2){
                        ent.ip_next=adress;
                        ent.port_next=Integer.parseInt(tab[1]);
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
    
    public static InetAddress host_ip(){
        try{
            Enumeration<NetworkInterface> listNi=NetworkInterface.getNetworkInterfaces();
            InetAddress iac=null;
            while(listNi.hasMoreElements()){
                NetworkInterface nic=listNi.nextElement();
                Enumeration<InetAddress> listIa=nic.getInetAddresses();
                while(listIa.hasMoreElements()){
                    iac=listIa.nextElement();
                    if(iac instanceof Inet4Address && !iac.isLoopbackAddress()){
                        return iac;
                    }
                }
            }
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
        return null;
    }
    
    public static String message_id(){
        String id="";
        Random rand = new Random();
        long h=rand.nextInt(1000);
        long cofh;
        String s =Long.toString(new java.util.Date().getTime())+ent.ip+Integer.toString(ent.udp);
        for(int i=0;i<s.length();i++){
            h=h*33+s.charAt(i);
        }
        long k;
        for(int i=0;i<8;i++){
            k=h;
            if(k%62<0) k=(-2)*(k%62);
            cofh=(k%62)+48;
            if(cofh>57) cofh+=7;
            if(cofh>90) cofh+=6;
            id=id+ (char)(cofh);
            h=(int)(h/62);
        }
        return id;
    }
    
    public static String remove_zero(String s){
        if(s.charAt(0)=='0') return remove_zero(s.substring(1));
        return s;
    }
}
