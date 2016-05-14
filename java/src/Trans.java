import java.util.*;
import java.net.*;
import java.io.*;

public class Trans{
    String id_trans;
    String file_name;
    FileOutputStream fos;
    long num_mess;
    long nb_mess;
   
    
    public Trans(String i, String f,String nb_m){
        try{
            id_trans=i;
            file_name=f;
            fos=new FileOutputStream(new File(file_name));
            num_mess=0;
            nb_mess=Long.parseLong(nb_m);
        }catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
    }
    
    public static int search(ArrayList<Trans> trans_list,String id){
        for(int i=0;i<trans_list.size();i++){
            if(trans_list.get(i).id_trans.equals(id)) return i;
        }
        return -1;
    }
}
