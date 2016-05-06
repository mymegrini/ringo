import java.util.*;
import java.net.*;
import java.io.*;

public class Diff_thread implements Runnable{
    Entity ent;
    String ip;
    int port;
    DatagramSocket dso;
    DatagramSocket dso_udp;
    ServerSocket server_tcp;

    public Diff_thread(Entity e,int d){
        try{
            ent=e;
            dso = new DatagramSocket(port);
            if(d==1){
                ip=ent.mdiff_ip;
                port=ent.mdiff_port;
            }
            else{
                ip=ent.mdiff_ip2;
                port=ent.mdiff_port2;
            }
        }catch(SocketException f){
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
                    dso.receive(packet_recv);
                    mess_recv = new String(packet_recv.getData(),0,packet_recv.getLength());
                    System.out.println("Diff message : "+mess_recv);
                    if(mess_recv.equals("DOWN")){
                        if(ent.port_next2!=-1){
                            if(port==ent.mdiff_port){
                                ent.port_next=ent.port_next2;
                                ent.ip_next=ent.ip_next2;
                                ent.port_next2=-1;
                            }
                            else ent.port_next2=-1;
                        }
                        else{
                            dso_udp.close();
                            server_tcp.close();
                        }
                        break;
                    }
                }catch(Exception e){
                    System.out.println("Fin du diff");
                    break;
                }
            }
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
