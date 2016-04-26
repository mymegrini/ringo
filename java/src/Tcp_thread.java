import java.util.*;
import java.net.*;
import java.io.*;

public class Tcp_thread implements Runnable{
    Entity ent;
    ServerSocket server;

    public Tcp_thread(Entity e){
        ent=e;
    }
    
    public void run(){
        try{
            server = new ServerSocket(ent.tcp);
            boolean dupl = false;
            Socket socket;
            while(true){
                socket = server.accept();
                if(Thread.currentThread().isInterrupted()){
                    socket.close();
                    server.close();
                    System.out.println("sort");
                    break;
                }
                BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
                if(!dupl){
                    String mess = "WELC "+ent.ip_next+" "+ent.port_next+" "+ent.mdiff_ip+" "+ent.mdiff_port;
                    pw.println(mess);
                    pw.flush();
                    mess=br.readLine();
                    Newc_mess data_n = Newc_mess.parse_newc(mess);
                    if(data_n!=null){
                        ent.ip_next=data_n.ip;
                        ent.port_next=data_n.port;
                        pw.println("ACKC");
                        pw.flush();
                    }
                    Dupl_mess data_d = Dupl_mess.parse_dupl(mess);
                    if(data_d!=null){
                        ent.ip_next2=data_d.ip2;
                        ent.port_next2=data_d.port2;
                        ent.mdiff_ip2=data_d.mdiff_ip2;
                        ent.mdiff_port2=data_d.mdiff_port2;
                        pw.println("ACKD");
                        pw.flush();
                        dupl=true;
                    }
                }
                else {
                    pw.println("NOTC");
                    pw.flush();
                }
                br.close();
                pw.close();
                socket.close();
            }
            System.out.println("sort");
        }catch(Exception e){
            System.out.println("Fin du tcp");
            //System.out.println(e);
            e.printStackTrace();
        }
    }

}
