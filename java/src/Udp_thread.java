import java.util.*;
import java.net.*;
import java.io.*;

public class Udp_thread implements Runnable{
    Entity ent;
    static ArrayList<String> mess_list;
    boolean quit;
    static ArrayList<Trans> trans_list;
    DatagramSocket dso;

    public Udp_thread(Entity e,ArrayList<String> list,DatagramSocket dso_udp){
        ent=e;
        mess_list=list;
        quit=false;
        trans_list=new ArrayList<Trans>();
        dso=dso_udp;
    }
    
    public void run(){
        try{
            byte[] data = new byte[512];
            DatagramPacket packet_recv = new DatagramPacket(data,data.length);
            String mess_recv;
            String mess_send;
            String []tab;
            String mess_id;
            int nb_ring=1;
            FileOutputStream fos = null;
            byte[] data_fic = new byte[469];
            while(true){
                try{
                    dso.receive(packet_recv);
                    mess_recv = new String(packet_recv.getData(),0,packet_recv.getLength());
                    System.out.println("Ring message : "+mess_recv);
                    tab = mess_recv.split(" ");
                    if(tab.length==2 && tab[0].equals("WHOS")){
                        if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv);
                        if(!mess_list.contains(tab[1])){
                            mess_id =Jring.message_id();
                            mess_send="MEMB "+mess_id+" "+ent.id+" "+ent.ip+" "+ent.udp;
                            mess_list.add(mess_id);
                            send_mess(ent,dso,mess_send);
                        }
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
                                if(ent.ip_next2.equals(tab[4]) && ent.port_next2==Integer.parseInt(tab[5])) ent.port_next2=-1;
                                else{
                                    ent.ip_next2=tab[4];
                                    ent.port_next2=Integer.parseInt(tab[5]);
                                }
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
                            if(!mess_list.remove(tab[1])){ send_mess(ent,dso,mess_recv);}
                            else System.out.println("TEST : Ring Check");
                        }
                    }
                    if(tab.length==5 && tab[0].equals("APPL") && tab[2].equals("DIFF####")){
                        if(!mess_list.remove(tab[1]))  send_mess(ent,dso,mess_recv);
                    }
                    if(tab[0].equals("APPL") && tab[2].equals("TRANS###")){
                        boolean r=mess_list.remove(tab[1]);
                        if(tab.length==6 && tab[3].equals("REQ")){
                            if(!r){
                                FileInputStream fis=null;
                                try{
                                    File file =new File(tab[5]);
                                    long t=file.length();
                                    fis =new FileInputStream(file);
                                    String id_trans = Jring.message_id();
                                    mess_send="APPL "+tab[1]+"TRANS### "+"ROK "+id_trans+" "+tab[4]+" "+tab[5]+" "+(int)(t/469);
                                    send_mess(ent,dso,mess_send);
                                    int i=0;
                                    while(fis.read(data_fic)>=0){
                                        mess_id=Jring.message_id();
                                        mess_list.add(mess_id);
                                        mess_send="APPL "+mess_id+"TRANS### "+"SEN "+id_trans+" "+i+" "+data_fic.length+" "+data_fic;
                                        send_mess(ent,dso,mess_send);
                                        i++;
                                    }
                                }
                                catch(FileNotFoundException e){
                                }
                                catch(IOException e){
                                    //System.out.println(e);
                                    e.printStackTrace();
                                }finally{
                                    try{
                                        if(fis!=null) fis.close(); 
                                    }
                                    catch(IOException e){ 
                                        e.printStackTrace();
                                    }
                                }
                            }
                            else System.out.println("The file is not present in the ring");
                        }
                        try{
                            if(tab.length==8 && tab[3].equals("ROK")){
                                if(r){
                                    trans_list.add(new Trans(tab[4],tab[6]));
                                    fos=new FileOutputStream(new File(tab[6]));
                                }
                                else send_mess(ent,dso,mess_recv);
                            }
                            int i =  Trans.search(trans_list,tab[4]);
                            if(tab.length==8 && tab[3].equals("SEN")){
                                if(i!=-1){
                                    if(trans_list.get(i).num_mess==Integer.parseInt(tab[5])){
                                        data_fic=new byte[469];
                                        data_fic = tab[7].getBytes();
                                        fos=new FileOutputStream(new File(trans_list.get(i).file_name));
                                        fos.write(data_fic);
                                        trans_list.get(i).num_mess++;
                                    }
                                    else{
                                        (new File(trans_list.get(i).file_name)).delete();
                                        mess_id=Jring.message_id();
                                        int size_mess = trans_list.get(i).file_name.length();
                                        mess_send= mess_send="APPL "+mess_id+" "+"TRANS### "+"REQ "+size_mess+" "+trans_list.get(i).file_name;
                                        send_mess(ent,dso,mess_send);
                                    }
                                }
                                else send_mess(ent,dso,mess_recv);
                            }
                        }catch(IOException e){
                            e.printStackTrace();
                        }
                        finally{
                            try{
                                if(fos!=null) fos.close();
                            }
                            catch(IOException e){
                                e.printStackTrace();
                            }
                        }
                    }
                }catch(SocketException e){
                    System.out.println(e);
                    break;
                }
            }
                
        }
        catch(Exception e){
            System.out.println(e);
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
