import java.util.*;
import java.net.*;
import java.io.*;

class TcpException extends Exception{
    
    public TcpException(){
        System.out.println("End of the Tcp connexion");
    }
}
