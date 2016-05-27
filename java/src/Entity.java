import java.math.*;

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
        id=add_space(ide,8);
        ip=ip_form(i);
        udp=u;
        tcp=t;
        mdiff_ip=ip_form(mdip);
        mdiff_port=mdpo;
        port_next2=-1;
    }
    
    public void all_info(){
        System.out.println("Identity "+id+" Ip "+ip);
        System.out.println("Port udp "+udp+" Port tcp "+tcp);
        System.out.println("Next ip "+ip_next+" Next port "+port_next);
        System.out.println("Diffusion ip "+mdiff_ip+" Diffusion port "+mdiff_port);
        if(port_next2!=-1){
            System.out.println("Ring 2 Next ip "+ip_next2+" Next port "+port_next2);
            System.out.println("Ring 2 Diffusion ip "+mdiff_ip2+" Diffusion port "+mdiff_port2);
        }
    }

    public static String ip_form(String ip){
        String []tab = ip.split("\\.");
        if(ip.length()==15) return ip;
        return add_zero(tab[0],3)+"."+add_zero(tab[1],3)+"."+add_zero(tab[2],3)+"."+add_zero(tab[3],3);
    }
    
    public static String add_space(String id,int length_final){
        int l=id.length();
        for(int i=0;i<length_final-l;i++){
            id=id.concat("#");
        }
        return id;
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

    public static String lend(long l){
        BigInteger n = BigInteger.valueOf(l);
        String lit="";
        BigInteger[] t = new BigInteger[2];
        for(int i=8;i>=0;i--){
            t = n.divideAndRemainder(BigInteger.valueOf((int)Math.pow(256,i)));
            lit=t[0].toString().concat(lit);
            n=t[1];
        }
        return lit;
    }
    
    public static long denl(String s){
        long k=0;
        for(int i=0;i<8;i++){
            k+=(int)Math.pow(256,i)*Integer.parseInt(String.valueOf(s.charAt(i)));
        }
        return k;
    }
    
}
