public class Entity{
    String id;
    int udp;
    int tcp;
    String ip_next;
    int port_next;
    String mdiff_ip;
    int mdiff_port;

    public Entity(String identifiant,int u,int t){
        id=identifiant;
        udp=u;
        tcp=t;
    }
    
}
