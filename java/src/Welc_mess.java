public class Welc_mess{
    String ip_next;
    int port_next;
    String mdiff_ip;
    int mdiff_port;

    public Welc_mess(String ip_n,int port_n,String md_ip,int md_port){
        ip_next=ip_n;
        port_next=port_n;
        mdiff_ip=md_ip;
        mdiff_port=md_port;
    }

    public static Welc_mess parse_welc(String mess){
        if(mess==null) return null;
        String []tab = mess.split(" ");
        if(mess.equals("NOTC")) System.out.println(mess);
        if(tab.length!=5 || !tab[0].equals("WELC")){
            System.out.println("parse_welc : The message doesn't have the right structure (1)");
            return null;
        }
        try{
            return new Welc_mess(tab[1],Integer.parseInt(tab[2]),tab[3],Integer.parseInt(tab[4]));
        }
        catch(Exception e){
            System.out.println("parse_welc : The message doesn't have the right structure (2)");
            return null;
        }
    }
}
