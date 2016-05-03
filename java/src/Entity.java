public class Entity{
    String id;
    String ip;
    int udp;
    int tcp;
    String ip_next;
    int port_next;
    String mdiff_ip;
    int mdiff_port;
    String ip_next2;
    int port_next2;
    String mdiff_ip2;
    int mdiff_port2;

    public Entity(String ide,String i,int u,int t,String mdip,int mdpo){
        id=ide;
        ip=Jring.ip_form(i);
        udp=u;
        tcp=t;
        mdiff_ip=Jring.ip_form(mdip);
        mdiff_port=mdpo;
        port_next2=-1;
    }
    
}
