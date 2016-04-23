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
        System.out.println("Give your id");
        Scanner sc = new Scanner(System.in);
        String id = sc.nextLine();
        while(id.length()>8){
            System.out.println("The maximum length of your id is 8 characters \n Give a new id");
            id = sc.nextLine();
        }
        System.out.println("Give your udp port");
        int u = sc.nextInt();
        System.out.println("Give your tcp port");
        int t = sc.nextInt();
        String mess_send = sc.nextLine();
        ent = new Entity(id,u,t);
        if(args.length==0){
            ent.ip=host_ip.toString().substring(1);
            ent.mdiff_ip="255.255.255.255";
            ent.mdiff_port=4444;
            ent.ip_next=ent.ip;
            ent.port_next=u;
        }
        else{            
            insert(args[0],Integer.parseInt(args[1]));
        }
        try{            
            //Tcp
            Tcp_thread tcp_mode = new Tcp_thread(ent);
            Thread tcp_t = new Thread(tcp_mode);
            tcp_t.start();
            //Udp
            Udp_thread udp_mode = new Udp_thread(ent,mess_list);
            Thread udp_t = new Thread(udp_mode);
            udp_t.start();
            //Send a message
            byte[] data = new byte[512];
            DatagramSocket dso = new DatagramSocket();
            DatagramPacket packet_send;
            Down_thread dwn;
            Thread dwn_t;
            while(true){                
                mess_send= sc.nextLine();
                if(udp_mode.quit) break;
                if(mess_send.equals("WHOS")){
                    mess_send="WHOS "+message_id();
                }
                if(mess_send.equals("GBYE")){
                    tcp_t.interrupt();
                    mess_send="GBYE "+message_id()+" "+ent.ip+" "+ent.udp+" "+ent.ip_next+" "+ent.port_next;
                }
                if(mess_send.equals("TEST")){
                    String m_id=message_id();
                    mess_send="TEST "+m_id+" "+ent.mdiff_ip+" "+ent.mdiff_port;
                    dwn = new Down_thread(ent,mess_list,m_id,2);
                    dwn_t = new Thread(dwn);
                    dwn_t.start();
                }
                mess_list.add(mess_send.split(" ")[1]);
                data=mess_send.getBytes();
                packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                dso.send(packet_send);
            }            
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
    

    public static void insert(String adress,int tcp){
        try{
            Socket socket = new Socket(adress,tcp);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            String mess_recv=br.readLine();
            Welc_mess data_welc = Welc_mess.parse_welc(mess_recv);
            if(data_welc!=null){ 
                ent.ip_next=data_welc.ip_next;
                ent.port_next=data_welc.port_next;
                ent.mdiff_ip=data_welc.mdiff_ip;
                ent.mdiff_port=data_welc.mdiff_port;
                String mess_send = "NEWC "+host_ip().toString().substring(1)+" "+ent.udp;
                pw.println(mess_send);
                pw.flush();
                mess_recv=br.readLine();
                if(mess_recv.equals("ACKC")){
                    System.out.println("insert : Successful Connexion !");
                }
            }
            else System.out.println("insert : Connexion Failed !");
            br.close();
            pw.close();
            socket.close();
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
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
        //long h = rand.nextInt((int)(Math.pow(2,16))-1);
        long h=rand.nextInt(1000);
        long cofh;
        //System.out.println(h);
        String s =Long.toString(new java.util.Date().getTime())+ent.ip+Integer.toString(ent.udp);
        //String s =Long.toString(new java.util.Date().getTime())+"192.168.1.145"+"8888";
        for(int i=0;i<s.length();i++){
            h=h*33+s.charAt(i);
        }
        for(int i=0;i<8;i++){
            cofh=(h%62)+48;
            //System.out.println("cof "+cofh);
            if(cofh>57) cofh+=7;
            if(cofh>90) cofh+=6;
            id=id+(char)(cofh);
            h=(int)(h/62);
        }
        return id;
    }
}
