import java.util.*;
import java.net.*;
import java.io.*;

public class Tcp_thread implements Runnable {
    Entity ent;

    public Tcp_thread(Entity e){
        ent=e;
    }
    
    public void run() {
        try{
            ServerSocket server = new ServerSocket(ent.tcp);
            while(true){
                Socket socket = server.accept();
                Service_tcp serv_tcp = new Service_tcp(socket,ent);
                Thread t = new Thread(serv_tcp);
                t.start();
            }
        }
        /*catch(InterruptedException e){
            //socket.close();
        }*/
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
