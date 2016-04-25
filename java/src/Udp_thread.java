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
            //DatagramSocket dso_diff = new DatagramSocket(ent.mdiff_port);
            byte[] data = new byte[512];
            DatagramPacket packet_recv = new DatagramPacket(data,data.length);
            //DatagramPacket packet_recv_diff = new DatagramPacket(data,data.length);
            String mess_recv;
            String mess_send;
            String []tab;
            String mess_id;
            int nb_ring=1;
            while(true){
                dso.receive(packet_recv);
                mess_recv = new String(packet_recv.getData(),0,packet_recv.getLength());
                System.out.println("Ring message : "+mess_recv);
                tab = mess_recv.split(" ");
                if(tab.length==2 && tab[0].equals("WHOS")){
                    if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv,nb_ring);
                    mess_id =Jring.message_id();
                    mess_send="MEMB "+mess_id+" "+ent.id+" "+ent.ip+" "+ent.udp;
                    send_mess(ent,dso,mess_send,nb_ring);
                    mess_list.add(mess_id);
                }
                if(tab.length==5 && tab[0].equals("MEMB")){
                    if(!mess_list.remove(tab[1]))  send_mess(ent,dso,mess_recv,nb_ring);
                }
                if(tab.length==6 && tab[0].equals("GBYE")){
                    if(ent.ip_next.equals(tab[2]) && ent.port_next==Integer.parseInt(tab[3])){
                        mess_id =Jring.message_id();
                        mess_send="EYBG "+mess_id;
                        send_mess(ent,dso,mess_send,nb_ring);
                        ent.ip_next=tab[4];
                        ent.port_next=Integer.parseInt(tab[5]);
                    }
                    else{
                        System.out.println("mess envoyé "+mess_recv);
                        send_mess(ent,dso,mess_recv,nb_ring);
                    }
                }
                if(tab.length==2 && tab[0].equals("EYBG")){
                    quit=true;
                    break;
                }
                if(tab.length==4 && tab[0].equals("TEST")){
                    if(tab[2].equals(ent.mdiff_ip) && Integer.parseInt(tab[3])==ent.mdiff_port){
                        if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv,1);
                        else System.out.println("TEST : Ring Check");
                    }
                    if(tab[2].equals(ent.mdiff_ip2) && Integer.parseInt(tab[3])==ent.mdiff_port2){
                        if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv,2);
                        else System.out.println("TEST : Ring Check");
                    }
                }
                if(tab.length==1 && tab[0].equals("DOWN")){
                    if(nb_ring==1 || nb_ring==2){
                        quit=true;
                        break;
                    }
                    else{
                        if(packet_recv.getAddress().toString().substring(1).equals(ent.mdiff_ip) && packet_recv.getPort()==ent.mdiff_port) nb_ring=2;
                        else nb_ring=1;
                    }
                }
                if(nb_ring==0) mess_list.add(tab[1]);
            }
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
    
    public static void send_mess(Entity ent,DatagramSocket dso,String mess,int d){
        try{
            byte[] data = mess.getBytes();
            DatagramPacket packet_send;
            DatagramPacket packet_send2;
            if(d==0){
                packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                packet_send2 = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next2,ent.port_next2));
                dso.send(packet_send);
                dso.send(packet_send2);
            }
            if(d==1){
                packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
                dso.send(packet_send);                
            }
            if(d==2) {
                packet_send2 =new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next2,ent.port_next2));
                dso.send(packet_send2);
            }
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
