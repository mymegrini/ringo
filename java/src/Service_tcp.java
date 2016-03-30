import java.util.*;
import java.net.*;
import java.io.*;

public class Service_tcp implements Runnable{
    Socket socket;
    Entity ent;
    
    public Service_tcp(Socket s,Entity e){
        socket=s;
        ent=e;
    }

    public synchronized void run(){
        try{
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            String mess = "WELC "+ent.ip_next+" "+ent.port_next+" "+ent.mdiff_ip+" "+ent.mdiff_port;
            pw.println(mess);
            pw.flush();
            mess=br.readLine();
            //System.out.println(mess);
            Newc_mess data = Newc_mess.parse_newc(mess);
            if(data!=null){
                ent.ip_next=data.ip;
                ent.port_next=data.port;
                pw.print("ACKC\n");
                pw.flush();
            }
            br.close();
            pw.close();
            socket.close();
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
