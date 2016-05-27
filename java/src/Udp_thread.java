import java.util.*;
import java.net.*;
import java.io.*;
import java.math.*;

public class Udp_thread implements Runnable{
    Entity ent;
    static ArrayList<String> mess_list;
    static ArrayList<Trans> trans_list;
    DatagramSocket dso;
    MulticastSocket mso;
    

    public Udp_thread(Entity e,ArrayList<String> list,DatagramSocket dso_u,MulticastSocket mso_d){
        ent=e;
        mess_list=list;
        trans_list=new ArrayList<Trans>();
        dso =  dso_u;
        mso=mso_d;
    }
    
    public void run(){
        try{
            byte[] data = new byte[512];
            DatagramPacket packet_recv = new DatagramPacket(data,data.length);
            String mess_recv;
            String []tab;
            int nb_ring=1;
            FileOutputStream fos = null;
            while(true){
                try{
                    dso.receive(packet_recv);
                    mess_recv = new String(packet_recv.getData(),0,packet_recv.getLength());
                    tab = mess_recv.split(" ");
                    System.out.println("Ring message : "+mess_recv);
                    mess_recv_WHOS(tab,mess_recv);
                    mess_recv_MEMB(tab,mess_recv);
                    mess_recv_GBYE(tab,mess_recv);
                    mess_recv_EYBG(tab);
                    mess_recv_TEST(tab,mess_recv);
                    mess_recv_DIFF(tab,mess_recv);
                    mess_recv_TRANS(tab,mess_recv);
                }catch(Exception e){
                    System.out.println("Fin du udp");
                    break;
                }
            }
        }
        catch(Exception e){
            e.printStackTrace();
            System.out.println(e);
        }
    }

    public void mess_recv_WHOS(String[] tab,String mess_recv){
        if(tab.length==2 && tab[0].equals("WHOS")){
            if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv);
            if(!mess_list.contains(tab[1])){
                String mess_id =Jring.message_id();
                String mess_send="MEMB "+mess_id+" "+ent.id+" "+ent.ip+" "+ent.udp;
                mess_list.add(mess_id);
                send_mess(ent,dso,mess_send);
            }
        }
    }
    
    public void mess_recv_MEMB(String[] tab,String mess_recv){
        if(tab.length==5 && tab[0].equals("MEMB")){
            if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv);
        }
    }
    public void mess_recv_GBYE(String[] tab,String mess_recv){
        if(tab.length==6 && tab[0].equals("GBYE")){
            String mess_send;
            String mess_id;
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
    }

    public void mess_recv_EYBG(String[] tab){
        try{
            if(tab.length==2 && tab[0].equals("EYBG") && mess_list.remove(tab[1])){
                mso.leaveGroup(InetAddress.getByName(ent.mdiff_ip));
                if(ent.port_next2!=-1) mso.leaveGroup(InetAddress.getByName(ent.mdiff_ip2));
                dso.close();
                System.exit(0);
                //System.out.println("Write quit to leave");
            }
        }catch(Exception e){
            System.out.println(e);
        }
    }
    
    public void mess_recv_TEST(String[] tab,String mess_recv){
        if(tab.length==4 && tab[0].equals("TEST")){
            if(tab[2].equals(ent.mdiff_ip) && Integer.parseInt(tab[3])==ent.mdiff_port || tab[2].equals(ent.mdiff_ip2) && Integer.parseInt(tab[3])==ent.mdiff_port2){
                if(!mess_list.remove(tab[1])) send_mess(ent,dso,mess_recv);
                else System.out.println("TEST : Ring "+tab[2]+" "+tab[3]+" Check");
            }
        }
    }
    
    public void mess_recv_DIFF(String[] tab,String mess_recv){
        if(tab.length==5 && tab[0].equals("APPL") && tab[2].equals("DIFF####")){
            if(!mess_list.remove(tab[1]))  send_mess(ent,dso,mess_recv);
        }
    }

    public void mess_recv_TRANS(String[] tab,String mess_recv){
        if(tab[0].equals("APPL") && tab[2].equals("TRANS###")){
            boolean r=mess_list.remove(tab[1]);
            String mess_send;
            String mess_id;
            byte[] data_fic = new byte[469];
            if(tab.length==6 && tab[3].equals("REQ")){
                if(!r){
                    FileInputStream fis=null;
                    try{
                        File file =new File(tab[5]);
                        long t=file.length();
                        fis =new FileInputStream(file);
                        String id_trans = Jring.message_id();
                        long nb_mess =(long)(t/469);
                        if(t%469!=0) nb_mess++;
                        mess_send="APPL "+tab[1]+" TRANS### "+"ROK "+id_trans+" "+Entity.add_zero(tab[4],2)+" "+tab[5]+" "+Entity.lend(nb_mess);
                        send_mess(ent,dso,mess_send);
                        long i=0;
                        int len=fis.read(data_fic);
                        while(len>=0){
                            mess_id=Jring.message_id();
                            mess_list.add(mess_id);
                            mess_send="APPL "+mess_id+" TRANS### "+"SEN "+id_trans+" "+Entity.lend(i)+" "+Entity.add_zero(len,3)+" "+new String(data_fic);
                            send_mess(ent,dso,mess_send);
                            i++;
                            data_fic=new byte[469];
                            len=fis.read(data_fic);
                        }
                    }
                    catch(FileNotFoundException e){
                        send_mess(ent,dso,mess_recv);                        
                    }
                    catch(IOException e){
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
                else System.out.println("The file "+tab[5]+" is not present in the ring");
            }
            if(tab.length==8 && tab[3].equals("ROK")){
                if(r) trans_list.add(new Trans(tab[4],tab[6],tab[7]));
                else send_mess(ent,dso,mess_recv);
            }
            int i =  Trans.search(trans_list,tab[4]);
            if(tab.length>=8 && tab[3].equals("SEN")){
                tab = mess_recv.split(" ",8);
                if(i!=-1){
                    if(trans_list.get(i).num_mess==Entity.denl(tab[5])){
                        try{
                            data_fic=new byte[469];
                            data_fic = tab[7].getBytes();
                            trans_list.get(i).fos.write(data_fic);
                            trans_list.get(i).num_mess++;
                            if(trans_list.get(i).num_mess+1==trans_list.get(i).nb_mess){
                                System.out.println("Transfer Complete");
                                trans_list.get(i).fos.close();
                                trans_list.remove(i);
                            }
                        }catch(IOException e){
                            e.printStackTrace();
                        }
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
        }
    }

    public static void send_mess(Entity ent,DatagramSocket dso,String mess){
        try{
            byte[] data = mess.getBytes();
            DatagramPacket packet_send;
            packet_send = new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next,ent.port_next));
            dso.send(packet_send);
            if(ent.port_next2!=-1){
                String[] tab = mess.split(" ");
                if(!tab[0].equals("TEST")) mess_list.add(tab[1]);
                packet_send =new DatagramPacket(data,data.length,new InetSocketAddress(ent.ip_next2,ent.port_next2));
                dso.send(packet_send);
            }
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
