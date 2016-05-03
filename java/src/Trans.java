import java.util.*;
import java.net.*;
import java.io.*;

public class Trans{
    String id_trans;
    String file_name;
    int num_mess;
    
    public Trans(String i, String f){
        id_trans=i;
        file_name=f;
        num_mess=0;
    }
    
    public static int search(ArrayList<Trans> trans_list,String id){
        for(int i=0;i<trans_list.size();i++){
            if(trans_list.get(i).id_trans.equals(id)) return i;
        }
        return -1;
    }
}
