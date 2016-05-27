import java.util.*;
import java.net.*;
import java.io.*;

public class Down_thread implements Runnable{
    Entity ent;
    ArrayList<String> mess_list;
    String mess_id;
    int nb_test;
    String ip_diff;
    int port_diff;

    public Down_thread(Entity e,ArrayList<String> list,String id,int nb,String ip,int port){
        ent=e;
        mess_list=list;
        mess_id=id;
        nb_test=nb;
        ip_diff=ip;
        port_diff=port;
    }
    
    public void run(){
        try{
            String mess_send;
            byte[] data = new byte[512];
            DatagramSocket dso = new DatagramSocket();
            DatagramPacket packet_send;
            boolean found = false;
            for(int i=0;i<nb_test;i++){
                Thread.sleep(15000);
                if(mess_list.remove(mess_id)){
                    System.out.println("TEST : The ring is not safe \n The test is restart to be sure !");
                    mess_id=Jring.message_id();
                    mess_send="TEST "+mess_id+" "+ip_diff+" "+port_diff;
                    mess_list.add(mess_id);
                    data=mess_send.getBytes();
                    packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ip_diff,port_diff));
                    dso.send(packet_send);
                }
                else {
                    found=true;
                    break;
                }
            }
            if(!found){
                Thread.sleep(15000);
                if(mess_list.remove(mess_id)){
                    mess_send="DOWN";
                    data=mess_send.getBytes();
                    packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ip_diff,port_diff));
                    dso.send(packet_send);
                }
            }          
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
