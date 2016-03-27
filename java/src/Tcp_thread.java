import java.util.*;
import java.net.*;
import java.io.*;

public class Tcp_thread implements Runnable{
    ServerSocket server;
    Entity ent;

    public Tcp_thread(ServerSocket serv,Entity e){
        server=serv;
        ent=e;
    }
    
    public void run(){
        try{
            while(true){
                Socket socket = server.accept();
                Service_tcp serv_tcp = new Service_tcp(socket,ent);
                Thread t = new Thread(serv_tcp);
                t.start();
            }
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
}
