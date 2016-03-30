public class Gbye_mess{
    String idm;
    String ip;
    int port;
    String ip_next;
    int port_next;

    public Gbye_mess(String _idm,String ip_s,int port_s,String ip_n,int port_n){
        idm=_idm;
        ip=ip_s;
        port=port_s;
        ip_next=ip_n;
        port_next=port_n;
    }

    public static Gbye_mess parse_gbye(String[] tab,boolean need_checking){
        if(!need_checking) return new Gbye_mess(tab[1],tab[2],Integer.parseInt(tab[3]),tab[4],Integer.parseInt(tab[5]));
        int port,port_next;
        if(tab.length!=6 || !tab[0].equals("GBYE")){
            System.out.println("parse_gbye : The message doesn't have the right structure");
            return null;
        }
        if(tab[1].length()>8){
            System.out.println("parse_gbye : The identify of a message have 8 characters maximum");
            return null;
        }
        try{
            port=Integer.parseInt(tab[3]);
            port_next=Integer.parseInt(tab[5]);
        }
        catch(Exception e){
            System.out.println("parse_gbye : The message doesn't have the right structure, you don't give number for port and/or port_next'");
            return null;
        }
        return new Gbye_mess(tab[1],tab[2],port,tab[4],port_next);
    }
}
