import java.util.*;
import java.net.*;
import java.io.*;

public class Down_thread implements Runnable{
    Entity ent;
    ArrayList<String> mess_list;
    String mess_id;
    int nb_test;
    
    public Down_thread(Entity e,ArrayList<String> list,String id,int nb){
        ent=e;
        mess_list=list;
        mess_id=id;
        nb_test=nb;
    }
    
    public void run(){
        try{
            String mess_send;
            byte[] data = new byte[512];
            DatagramSocket dso = new DatagramSocket();
            DatagramPacket packet_send;
            boolean found = false;
            for(int i=0;i<nb_test;i++){
                Thread.sleep(15);
                if(Udp_thread.search(mess_id)!=-1){
                    mess_list.remove(mess_id);
                    mess_id=Jring.message_id();
                    mess_send="TEST "+mess_id+" "+ent.mdiff_ip+" "+ent.mdiff_port;
                    mess_list.add(mess_send.split(" ")[1]);
                    data=mess_send.getBytes();
                    packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                    dso.send(packet_send);
                }
                else found=true;
            }
            if(!found){
                Thread.sleep(15);
                if(Udp_thread.search(mess_id)!=-1){
                    mess_send="DOWN";
                    data=mess_send.getBytes();
                    packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.mdiff_ip,ent.mdiff_port));
                    dso.send(packet_send);
                }
            }
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
