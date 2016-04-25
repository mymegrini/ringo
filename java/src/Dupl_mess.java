public class Dupl_mess{
    String ip2;
    int port2;
    String mdiff_ip2;
    int mdiff_port2;
    
    public Dupl_mess(String i,int p,String i2,int p2){
        ip2=i;
        port2=p;
        mdiff_ip2=i2;
        mdiff_port2=p2;
    }

    public static Dupl_mess parse_dupl(String mess){
        String []tab = mess.split(" ");
        if(!tab[0].equals("DUPL")) return null;
        if(tab.length!=5){
            System.out.println("pase_dupl : The message doesn't have the right structure (1)");
            return null;
        }
        try{
            return new Dupl_mess(tab[1],Integer.parseInt(tab[2]),tab[3],Integer.parseInt(tab[4]));
        }
        catch(Exception e){
            System.out.println("parse_dupl : The message doesn't have the right structure (2)");
            return null;
        }
    }
}
