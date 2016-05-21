/** MessageManager .java
 */ 

import java.util.*;
import java.net.*;

public class MessageManager
{
  ////////////////////////////////////////////////////////////////////////////////
  // Internal classes
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Runnable class that listens to udp socket and trigger messages' actions
   */
  private class MessageAdministrator implements Runnable {

    private DatagramSocket  listenSock;
    private ArrayList<MessageAction> action;

    public MessageAdministrator(DatagramSocket _listenSock) { 
      listenSock = _listenSock;
      action = new ArrayList<>();
    }

    public void run() {
      byte[] b = new byte[512];
      DatagramPacket received = new DatagramPacket(b, 512);
      while (true) {
        try {
          listenSock.receive(received);
          String message = new String(b, 0, received.getLength());
          jring.verbose.println("Message received: " + message);
          String parsed[] = parseMsg(message);
          if (parsed == null)
            continue;
          String messageType = parsed[0], idm = parsed[1], content = parsed[2];
          if (listAdmin.add(idm))
            for (MessageAction a : action) {
              if (messageType.equals(a.getType())) {
                a.execute(message, content);
                break;
              }
            }
          else
            jring.verbose.println("Message not supported: " + message);
        } catch (Exception e) {
          jring.verbose.println("UDP RECEIVED EXCEPTION");
        }
      }
    }
  }


  /**
   * Manage the list of seen messages
   */
  private class MessageListAdministrator {

    private ArrayList<String> savedMessage;

    public MessageListAdministrator() {
      savedMessage = new ArrayList<>();
    }


    public synchronized boolean add(String idm) {
      if (savedMessage.indexOf(idm) == -1)
        return false;
      return savedMessage.add(idm);
    }
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Attributes
  ////////////////////////////////////////////////////////////////////////////////

  private Entity                   ent;
  private MessageAdministrator     messAdmin;
  private MessageListAdministrator listAdmin;
  private final Jring              jring;
  ////////////////////////////////////////////////////////////////////////////////
  // Public
  ////////////////////////////////////////////////////////////////////////////////

  public MessageManager (Jring _jring) throws Exception {
    jring     = _jring;
    ent       = jring.ent;
    messAdmin = new MessageAdministrator(ent.getListenSock());
    listAdmin = new MessageListAdministrator();
  }


  public void sendMessage(String message) {
    byte[] b = Jring.padString(message, 512).getBytes();
    ent.sendPacketAll(message);
  }


  public void sendMessage(String type, String content) {
    String idm = messageId();
    String message = jring.padString(type + " " + idm + " " + content, 512);
    listAdmin.add(idm);
    sendMessage(message);
  }


  public void sendAppMessage(String idApp, String content) {
    sendMessage("APPL", idApp + " " + content);
  }
  ////////////////////////////////////////////////////////////////////////////////
  // Private
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Return a String[3]:
   * 1. type
   * 2. id
   * 3. content
   */
  public static String[] parseMsg(String message) {
    if (message.charAt(4) != ' ' || message.charAt(13) != ' ') 
      return null;
    String[] parsed = new String[3];
    parsed[0] = message.substring(0, 4);
    parsed[1] = message.substring(5, 13);
    parsed[2] = message.substring(13,message.length());
    return parsed;
  }


  /**
   * Return a new message id
   */
  private String messageId(){
    String id="";
    Random rand = new Random();
    long h=rand.nextInt(1000);
    long cofh;
    String s = Long.toString(new java.util.Date().getTime())+ent.ip+Integer.toString(ent.udp);
    for(int i=0;i<s.length();i++){
      h=h*33+s.charAt(i);
    }
    long k;
    for(int i=0;i<8;i++){
      k=h;
      if(k%62<0) k=(-2)*(k%62);
      cofh=(k%62)+48;
      if(cofh>57) cofh+=7;
      if(cofh>90) cofh+=6;
      id=id+ (char)(cofh);
      h=(int)(h/62);
    }
    return id;
  }

}
//MessageManager .java 

