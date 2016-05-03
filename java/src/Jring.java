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
        ent = new Entity(id,host_ip.toString().substring(1),u,t,"255.255.255.255",4444);
        boolean stage1=true;
        if(args.length==0){
            ent.ip_next=ent.ip;
            ent.port_next=u;
        }
        else{            
            if(args.length==3){
                if(args[0].equals("insert")) stage1=insert(args[1],Integer.parseInt(args[2]));
                if(args[0].equals("duplicate")) stage1=duplicate(args[1],Integer.parseInt(args[2]));
            }
            else stage1=false;
        }
        if(stage1){
            try{
                //Tcp
                Tcp_thread tcp_mode = new Tcp_thread(ent);
                Thread tcp_t = new Thread(tcp_mode);
                tcp_t.start();
                //Udp
                DatagramSocket dso_udp =  new DatagramSocket(ent.udp);
                Udp_thread udp_mode = new Udp_thread(ent,mess_list,dso_udp);
                Thread udp_t = new Thread(udp_mode);
                udp_t.start();
                dso_udp.close();
                //Diff
                Diff_thread diff_mode = new Diff_thread(ent,1,dso_udp);
                Thread diff_t = new Thread(diff_mode);
                diff_t.start();
                //Send a message
                byte[] data = new byte[512];
                DatagramSocket dso = new DatagramSocket();
                DatagramPacket packet_send;
                Down_thread dwn;
                Thread dwn_t;
                String m_id;
                int size_mess;
                boolean f=true;
                while(true){
                    mess_send= sc.nextLine();
                    if(udp_mode.quit) break;
                    m_id=message_id();
                    if(mess_send.equals("WHOS")){
                        mess_send="WHOS "+m_id;
                    }
                    if(mess_send.equals("GBYE")){
                        tcp_mode.server.close();
                        mess_send="GBYE "+m_id+" "+ent.ip+" "+ent.udp+" "+ent.ip_next+" "+ent.port_next;
                        if(ent.port_next2!=-1){
                            mess_list.add(m_id);
                            Udp_thread.send_mess(ent,dso,mess_send);
                            m_id=message_id();
                            mess_send="GBYE "+m_id+" "+ent.ip+" "+ent.udp+" "+ent.ip_next2+" "+ent.port_next2;
                        }
                    }
                    if(mess_send.equals("TEST")){
                        mess_send="TEST "+m_id+" "+ent.mdiff_ip+" "+ent.mdiff_port;
                        dwn = new Down_thread(ent,mess_list,m_id,0);
                        dwn_t = new Thread(dwn);
                        dwn_t.start();
                    }
                    if(mess_send.equals("DIFF")){
                        System.out.println("Give your message (less than 512-x characters)");
                        mess_send= sc.nextLine();
                        size_mess=mess_send.length();
                        if(size_mess<486) mess_send="APPL "+m_id+" "+"DIFF#### "+size_mess+" "+mess_send;
                        else mess_send = "";
                    }
                    if(mess_send.equals("TRANS")){
                        System.out.println("Give the name of the file");
                        mess_send= sc.nextLine();
                        size_mess=mess_send.length();
                        if(size_mess<482) mess_send="APPL "+m_id+" "+"TRANS### "+"REQ "+size_mess+" "+mess_send;
                        else mess_send = "";
                    }
                    if(mess_send.equals("")){
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
                String mess_send = "NEWC "+ent.ip+" "+ent.udp;
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
            String ip_mdiff2 = "255.255.255.255";
            System.out.println("Give the port of the new ring");
            int port_mdiff2=sc.nextInt();
            Socket socket = new Socket(adress,tcp);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            String mess_recv=br.readLine();
            Welc_mess data_welc = Welc_mess.parse_welc(mess_recv);
            boolean dupl=false;
            if(data_welc!=null){
                String mess_send = "DUPL "+ent.ip+" "+ent.udp+" "+ip_mdiff2+" "+port_mdiff2;
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

    public static void diffuse(String mess,String m_id){
        
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
    
    public static String ip_form(String ip){
        String []tab = ip.split("\\.");
        if(ip.length()==15) return ip;
        return add_zero(tab[0],3)+"."+add_zero(tab[1],3)+"."+add_zero(tab[2],3)+"."+add_zero(tab[3],3);
    }
    
    /*public static String port_form(int port){
        
    }*/
    
    public static String add_zero(String s,int length_final){
        int l=s.length();
        for(int i=0;i<length_final-l;i++){
            s="0".concat(s);
        }
        return s;
    }
    
    public static String remove_zero(String s){
        if(s.charAt(0)=='0') return remove_zero(s.substring(1));
        return s;
    }
}
