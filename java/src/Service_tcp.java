import java.util.*;
import java.net.*;
import java.io.*;

public class Service_tcp implements Runnable{
    Socket socket;
    entity ent;
    public Service_tcp(Socket s,entity e){
        socket=s;
        ent=e;
    }

    public synchronized void run(){
        try{
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            String mess = "WELC "+ent.ip_next+" "+ent.port_next+" "+ent.mdiff_ip+" "+ent.mdiff_port+"\n";
            pw.println(mess);
            pw.flush();
            mess=br.readLine();
            newc_mess data = parse_newc(mess);
            ent.ip_next=data.ip;
            ent.port_next=data.port;
            pw.println("ACKC\n");
            pw.flush();
            br.close();
            pw.close();
            socket.close();
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
}
    }

    public newc_mess parse_newc(String mess){
        mess=mess.substring(0,mess.length()-1);
        String []tab = mess.split(" ");
        if(tab.length!=3 || !tab[0].equals("NEWC")){
            System.out.println("The message doesn't have the right structure (1)");
            return null;
        }
        try{
            return new newc_mess(tab[1],Integer.parseInt(tab[2]));
        }
        catch(Exception e){
            System.out.println("The message doesn't have the right structure (2)");
            return null;
        }
    }
}
