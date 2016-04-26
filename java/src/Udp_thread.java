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
                Thread.sleep(10000);
                if(/*tab.length==2 && */tab[0].equals("WHOS")){
                    if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv+" "+ent.id);
                    mess_id =Jring.message_id();
                    mess_send="MEMB "+mess_id+" "+ent.id+" "+ent.ip+" "+ent.udp;
                    mess_list.add(mess_id);
                    send_mess(ent,dso,mess_send);
                }
                if(tab.length==5 && tab[0].equals("MEMB")){
                    if(!mess_list.remove(tab[1]))  send_mess(ent,dso,mess_recv);
                }
                if(tab.length==6 && tab[0].equals("GBYE")){
                    if(ent.ip_next.equals(tab[2]) && ent.port_next==Integer.parseInt(tab[3])){
                        mess_id =Jring.message_id();
                        mess_send="EYBG "+tab[1];
                        send_mess(ent,dso,mess_send);
                        ent.ip_next=tab[4];
                        ent.port_next=Integer.parseInt(tab[5]);
                    }
                    else{
                        if(ent.ip_next2.equals(tab[2]) && ent.port_next2==Integer.parseInt(tab[3])){
                            mess_id =Jring.message_id();
                            mess_send="EYBG "+tab[1];
                            send_mess(ent,dso,mess_send);
                            ent.ip_next2=tab[4];
                            ent.port_next2=Integer.parseInt(tab[5]);
                        }
                        else send_mess(ent,dso,mess_recv);
                    }
                }
                if(tab.length==2 && tab[0].equals("EYBG") && mess_list.remove(tab[1])){
                    quit=true;
                    break;
                }
                if(tab.length==4 && tab[0].equals("TEST")){
                    if(tab[2].equals(ent.mdiff_ip) && Integer.parseInt(tab[3])==ent.mdiff_port || tab[2].equals(ent.mdiff_ip2) && Integer.parseInt(tab[3])==ent.mdiff_port2){
                        if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv);
                        else System.out.println("TEST : Ring Check");
                    }
                }
            }
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }

    
    public static void send_mess(Entity ent,DatagramSocket dso,String mess){
        try{
            byte[] data = mess.getBytes();
            DatagramPacket packet_send;
            packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
            dso.send(packet_send);
            if(ent.port_next2!=-1){
                mess_list.add(mess.split(" ")[1]);
                packet_send =new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next2,ent.port_next2));
                dso.send(packet_send);
            }
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
