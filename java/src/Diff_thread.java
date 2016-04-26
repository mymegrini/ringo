import java.util.*;
import java.net.*;
import java.io.*;

public class Diff_thread implements Runnable{
    Entity ent;
    int port;

    public Diff_thread(Entity e,int p){
        ent=e;
        port=p;
    }
    
    public void run(){
        try{
            DatagramSocket dso = new DatagramSocket(port);
            byte[] data = new byte[512];
            DatagramPacket packet_recv = new DatagramPacket(data,data.length);
            String mess_recv;
            while(true){
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
                    else break;              
                }
            }
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
