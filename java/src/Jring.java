import java.util.*;
import java.net.*;
import java.io.*;

public class Jring{
    static Entity ent;
    static ArrayList<String> mess_list;
    
    public static void main(String[] args){
        mess_list = new ArrayList<String>();
        InetAddress a =host_ip();
        System.out.println(a.toString());
        System.out.println("Give your id");
        Scanner sc = new Scanner(System.in);
        String id = sc.nextLine();
        while(id.length()>8){
            System.out.println("The maximum length of your id is 8 characters \n Give a new id");
            id = sc.nextLine();
        }
        System.out.println("Give your tcp port");
        int k = sc.nextInt();
        ent = new Entity(id,k);
        if(args.length==0){
            ent.mdiff_ip="255.255.255.255";
            ent.mdiff_port=4444;
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
            Udp_thread udp_mode = new Udp_thread(ent,a,mess_list);
            Thread udp_t = new Thread(udp_mode);
            udp_t.start();
            //Send a new message
            String mess_send;
            byte[] data = new byte[512];
            DatagramSocket dso = new DatagramSocket();
            InetSocketAddress next_ia = new InetSocketAddress(ent.ip_next,ent.port_next);
            DatagramPacket packet_send;
            while(true){
                mess_send=sc.nextLine();
                data=mess_send.getBytes();
                packet_send = new DatagramPacket(data,data.length,next_ia);
                dso.send(packet_send);
                String []tab = mess_send.split(" ");
                int type=type_mess(tab[0]);
                if(type==2){
                    if(parse_gbye(tab,true)!=null) mess_list.add(tab[0]);
                }              
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
            Welc_mess data_enter = parse_welc(mess_recv);
            if(data_enter!=null){ 
                ent.ip_next=data_enter.ip_next;
                ent.port_next=data_enter.port_next;
                ent.mdiff_ip=data_enter.mdiff_ip;
                ent.mdiff_port=data_enter.mdiff_port;
                String mess_send = "NEWC "+host_ip().toString()+" "+ent.udp;
                pw.println(mess_send);
                pw.flush();
                mess_recv=br.readLine();
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
    
    public static Welc_mess parse_welc(String mess){
        //System.out.println("avant sub "+mess);
        // mess=mess.substring(0,mess.length());
        //System.out.println("apres sub "+mess);
        String []tab = mess.split(" ");
        if(tab.length!=5 || !tab[0].equals("WELC")){
            System.out.println("parse_welc : The message doesn't have the right structure (1)");
            return null;
        }
        try{
            return new Welc_mess(tab[1],Integer.parseInt(tab[2]),tab[3],Integer.parseInt(tab[4]));
        }
        catch(Exception e){
            System.out.println("parse_welc : The message doesn't have the right structure (2)");
            return null;
        }
    }

    public static Gbye_mess parse_gbye(String[] tab,boolean need_checking){
        if(!need_checking) return new Gbye_mess(tab[1],tab[2],Integer.parseInt(tab[3]),tab[4],Integer.parseInt(tab[5]));
        int port,port_next;
        if(tab.length!=6 || !tab[0].equals("GBYE")){
            System.out.println("parse_gbye : The message doesn't have the right structure");
            return null;
        }
        if(tab[1].length()>8){
            System.out.println("parse_gbye : The identify of a message have 8 characters maximum");
            return null;
        }
        try{
            port=Integer.parseInt(tab[3]);
            port_next=Integer.parseInt(tab[5]);
        }
        catch(Exception e){
            System.out.println("parse_gbye : The message doesn't have the right structure, you don't give number for port and/or port_next'");
            return null;
        }
        return new Gbye_mess(tab[1],tab[2],port,tab[4],port_next);
    }

    public static int type_mess(String type){
        if(type.equals("WHOS")) return 1;
        if(type.equals("GBYE")) return 2;
        if(type.equals("Test")) return 3;
        return 0;
    }
}
