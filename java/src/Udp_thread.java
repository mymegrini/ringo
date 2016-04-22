import java.util.*;
import java.net.*;
import java.io.*;

public class Udp_thread implements Runnable{
    Entity ent;
    static ArrayList<String> mess_list;
    boolean quit;
    
    public Udp_thread(Entity e,ArrayList<String> list){
        ent=e;
        mess_list=list;
        quit=false;
    }
    
    public void run(){
        try{
            DatagramSocket dso = new DatagramSocket(ent.udp);
            byte[] data = new byte[512];
            DatagramPacket packet_recv = new DatagramPacket(data,data.length);
            DatagramPacket packet_send ;
            String mess_recv;
            String mess_send;
            String []tab;
            while(true){
                dso.receive(packet_recv);
                mess_recv = new String(packet_recv.getData(),0,packet_recv.getLength());
                System.out.println("Ring message : "+mess_recv);
                packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                tab = mess_recv.split(" ");
                if(tab.length==2 && tab[0].equals("WHOS")){
                    if(!mess_list.remove(tab[1])) dso.send(packet_send);
                    String mess_id =Jring.message_id();
                    mess_send="MEMB "+mess_id+" "+ent.id+" "+ent.ip+" "+ent.udp;
                    data=mess_send.getBytes();
                    packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                    dso.send(packet_send);
                    mess_list.add(mess_id);
                }
                if(tab.length==5 && tab[0].equals("MEMB")){
                    if(!mess_list.remove(tab[1])) dso.send(packet_send);
                }
                if(tab.length==6 && tab[0].equals("GBYE")){
                    if(ent.ip_next.equals(tab[2]) && ent.port_next==Integer.parseInt(tab[3])){
                        String mess_id =Jring.message_id();
                        mess_send="EYBG "+mess_id;
                        data=mess_send.getBytes();
                        packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                        dso.send(packet_send);
                        ent.ip_next=tab[4];
                        ent.port_next=Integer.parseInt(tab[5]);
                        break;
                    }
                    else{
                        dso.send(packet_send);
                    }
                }
                if(tab.length==2 && tab[0].equals("EYBG")){
                    quit=true;
                    break;
                }
                if(tab.length==4 && tab[0].equals("TEST")){
                    if(tab[2].equals(ent.mdiff_ip) && Integer.parseInt(tab[3])==ent.mdiff_port){
                        if(!mess_list.remove(tab[1]) ){
                            data=mess_recv.getBytes();
                            packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                            dso.send(packet_send);
                        }
                        else System.out.println("Test complete");
                    }
                }
                if(tab.length==1 && tab[0].equals("DOWN")){
                    quit=true;
                    break;
                }
            }
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }

    public static int search(String idm){
        for(int i=0;i<mess_list.size();i++){
            if(mess_list.get(i).equals(idm)) return i;
        }
        return -1;
    }
}
