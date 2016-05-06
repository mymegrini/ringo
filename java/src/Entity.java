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
        ip=ip_form(i);
        udp=u;
        tcp=t;
        mdiff_ip=ip_form(mdip);
        mdiff_port=mdpo;
        port_next2=-1;
    }

    public static String ip_form(String ip){
        String []tab = ip.split("\\.");
        if(ip.length()==15) return ip;
        return add_zero(tab[0],3)+"."+add_zero(tab[1],3)+"."+add_zero(tab[2],3)+"."+add_zero(tab[3],3);
    }
    
    public static String add_zero(int port,int length_final){
        String s=""+port;
        int l=s.length();
        for(int i=0;i<length_final-l;i++){
            s="0".concat(s);
        }
        return s;
    }
    
    

    public static String add_zero(String s,int length_final){
        int l=s.length();
        for(int i=0;i<length_final-l;i++){
            s="0".concat(s);
        }
        return s;
    }
    
}
