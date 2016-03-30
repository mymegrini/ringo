import java.util.*;
import java.net.*;
import java.io.*;

public class Udp_thread implements Runnable{
    Entity ent;
    InetAddress host_ip;
    static ArrayList<String> mess_list;

    public Udp_thread(Entity e,InetAddress h,ArrayList<String> list){
        ent=e;
        host_ip=h;
        mess_list=list;
    }
    
    public void run(){
        try{
            DatagramSocket dso = new DatagramSocket(ent.udp);
            byte[] data = new byte[512];
            DatagramPacket packet_recv = new DatagramPacket(data,data.length);
            //InetSocketAddress next_ia = ;
            DatagramPacket packet_send ;
            String mess_recv;
            String mess_send;
            while(true){
                dso.receive(packet_recv);
                mess_recv = new String(packet_recv.getData(),0,packet_recv.getLength());
                System.out.println("Ring message : "+mess_recv);
                packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                String []tab = mess_recv.split(" ");
                if(tab.length==2 && tab[0].equals("WHOS")){
                    if(!search(tab[1])){
                        //packet_send = new DatagramPacket(data,data.length,next_ia);
                        dso.send(packet_send);
                    }
                    mess_send="MEMB "+tab[1]+" "+ent.id+" "+host_ip.toString()+" "+ent.udp;
                    data=mess_send.getBytes();
                    packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                    dso.send(packet_send);
                }
                if(tab.length==5 && tab[0].equals("MEMB") && !search(tab[1])){
                    // packet_send = new DatagramPacket(data,data.length,next_ia);
                    dso.send(packet_send);
                }
                if(tab.length==6 && tab[0].equals("GBYE")){
                    if(ent.ip_next.equals(tab[2]) && ent.port_next==Integer.parseInt(tab[3])){
                        ent.ip_next=tab[4];
                        ent.port_next=Integer.parseInt(tab[5]);
                        mess_send="EYBG "+tab[1];
                        data=mess_send.getBytes();
                        packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                        dso.send(packet_send);
                    }
                    else{
                        //packet_send = new DatagramPacket(data,data.length,next_ia);
                        dso.send(packet_send);
                    }
                }
                if(tab.length==2 && tab[0].equals("EYBG")){
                    if(search(tab[1])){
                        break;
                    }
                    else{
                        //packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                        dso.send(packet_send);
                    }
                }
                /*if(tab.length==4 && tab[0].equals("TEST")){
                    if(tab[2].equals(ent.mdiff_ip) && Integer.parseInt(tab[3])==ent.mdiff_port){
                        packet_send = new DatagramPacket(data,data.length,next_ia);
                        dso.send(packet_send);
                    }
                }
                if(tab.length==1 && tab[0].equals("DOWN")){
                }*/
            }
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }

    public static boolean search(String idm){
        return true;
    }
}
