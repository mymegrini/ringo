import java.util.*;
import java.net.*;
import java.io.*;

public class Jring{
    static Entity ent;
    static ArrayList<String> mess_list;
    
    public static void main(String[] args) throws InterruptedException{
        boolean again=true;
        while(again){
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
                ent.mdiff_ip="255.255.255.255";
                ent.mdiff_port=4444;
                ent.ip_next=host_ip.toString().substring(1);
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
                Udp_thread udp_mode = new Udp_thread(ent,host_ip,mess_list);
                Thread udp_t = new Thread(udp_mode);
                udp_t.start();
                //Send a message
                byte[] data = new byte[512];
                DatagramSocket dso = new DatagramSocket();
                DatagramPacket packet_send;
                while(true){                
                    mess_send= sc.nextLine();
                    if(udp_mode.quit) break;
                    String []tab = mess_send.split(" ");
                    int type=type_mess(tab[0]);
                    if(type!=0){
                        data=mess_send.getBytes();
                        packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                        dso.send(packet_send);
                        mess_list.add(tab[1]);
                    }
                }
                //tcp_t.interrupt();
                //tcp_t.join();                
            }catch(Exception e){
                System.out.println(e);
                e.printStackTrace();
            }
            System.out.println("If you want to leave write quit Else press Enter");
            String n=sc.nextLine();
            if(n.equals("quit")) again=false;
        }
    }
    

    public static void insert(String adress,int tcp){
        try{
            Socket socket = new Socket(adress,tcp);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            String mess_recv=br.readLine();
            //System.out.println(mess_recv);
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
                //System.out.println(mess_recv);
                if(mess_recv.equals("ACKC")){
                    System.out.println("insert : Connexion Succeed !");
                }
            }
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
            boolean t=false;
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
    
    public static int type_mess(String type){
        if(type.equals("WHOS")) return 1;
        if(type.equals("GBYE")) return 2;
        if(type.equals("Test")) return 3;
        return 0;
    }
}
