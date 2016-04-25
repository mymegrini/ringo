public class Newc_mess{
    String ip;
    int port;
    
    public Newc_mess(String i,int p){
        ip=i;
        port=p;
    }

    public static Newc_mess parse_newc(String mess){
        String []tab = mess.split(" ");
        if( !tab[0].equals("NEWC")) return null;
        if(tab.length!=3){
            System.out.println("pase_newc : The message doesn't have the right structure (1)");
            return null;
        }
        try{
            return new Newc_mess(tab[1],Integer.parseInt(tab[2]));
        }
        catch(Exception e){
            System.out.println("parse_newc : The message doesn't have the right structure (2)");
            return null;
        }
    }
}
