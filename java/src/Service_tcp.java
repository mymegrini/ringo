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
            
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
