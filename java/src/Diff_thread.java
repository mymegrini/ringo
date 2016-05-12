import java.util.*;
import java.net.*;
import java.io.*;

public class Diff_thread implements Runnable{
    Entity ent;
    String ip;
    int port;
    MulticastSocket mso;
    DatagramSocket dso_udp;
    ServerSocket server_tcp;
    
    public Diff_thread(Entity e,String i,int p){
        try{
            ent=e;
            ip=i;
            port=p;
            mso = new MulticastSocket(port);
            mso.joinGroup(InetAddress.getByName(ip));
        }catch(Exception f){
            System.out.println(f);
        }
    }
    
    public void run(){
        try{
            byte[] data = new byte[512];
            DatagramPacket packet_recv = new DatagramPacket(data,data.length);
            String mess_recv;
            while(true){
                try{
                    mso.receive(packet_recv);
                    mess_recv = new String(packet_recv.getData(),0,packet_recv.getLength());
                    System.out.println("Diff message : "+mess_recv);
                    if(mess_recv.equals("DOWN")){
                        if(ent.port_next2!=-1){
                            if(port==ent.mdiff_port){
                                ent.port_next=ent.port_next2;
                                ent.ip_next=ent.ip_next2;
                                ent.mdiff_ip=ent.mdiff_ip2;
                                ent.port_next2=-1;
                            }
                            else ent.port_next2=-1;
                        }
                        else{
                            mso.leaveGroup(InetAddress.getByName(ip));
                            dso_udp.close();
                            server_tcp.close();
                            System.exit(0);
                        }
                        break;
                    }
                }catch(Exception e){
                    System.out.println(e);
                    e.printStackTrace();
                    System.out.println("Fin du diff");
                    break;
                }
            }
            System.out.println("Fin du diff");
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
