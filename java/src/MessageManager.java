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

    private MessageAction whos = new MessageAction() {
      public String getType() { return "WHOS"; }
      public int execute(String message, String content, boolean lookupFlag) {
        if (lookupFlag) {
          jring.verbose.println("Message " + message + " already seen, nothing to do.");
        } else {
          sendMessage("MEMB ", ent.paddedId + " " + ent.ip + " " + String.valueOf(ent.udp));
        }
        return 0;
      }
    };

    public MessageAdministrator(DatagramSocket _listenSock) { 
      listenSock = _listenSock;
      action = new ArrayList<>();

      // Supported actions
      action.add(whos);
    }

    public void run() {
      byte[] b = new byte[512];
      DatagramPacket received = new DatagramPacket(b, 512);
      while (true) {
        try {
          listenSock.receive(received);

          Thread treatAction = new Thread(new Runnable() {
            public void run() {
              String message = new String(b, 0, received.getLength());
              jring.verbose.println("Message received: " + message);
              String parsed[] = parseMsg(message);
              if (parsed == null)
                return;
              String messageType  = parsed[0], idm = parsed[1], content = parsed[2];
              boolean lookupFlag  = listAdmin.add(idm);
              boolean actionFound = false;
              for (MessageAction a : action) {
                if (messageType.equals(a.getType())) {
                  a.execute(message, content, lookupFlag);
                  actionFound = true;
                  break;
                }
              }
              if (!actionFound)
                jring.verbose.println("Message not supported: " + message);
            }
          });

          treatAction.start();

        } catch (Exception e) {
          jring.verbose.println("UDP RECEIVED EXCEPTION");
          System.out.println(e);
          e.printStackTrace();
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
  private Thread                   messAdminT;
  ////////////////////////////////////////////////////////////////////////////////
  // Public
  ////////////////////////////////////////////////////////////////////////////////

  public MessageManager (Jring _jring) throws Exception {
    jring     = _jring;
    ent       = jring.ent;
    messAdmin = new MessageAdministrator(ent.getListenSock());
    listAdmin = new MessageListAdministrator();
  }


  public void sendMessage(Entity.NextEntity next, String message) {
    message = jring.padString(message, 512);
    ent.sendPacket(next, " " + messageId() + " " + message);
  }


  public void sendMessage(Entity.NextEntity next, String type, String content) {
    String message = jring.padString(type + " " + content, 512);
    sendMessage(next, message);
  }


  public void sendMessage(String message) {
    message = jring.padString(message, 512);
    ent.sendPacketAll(message);
  }


  public void sendMessage(String type, String content) {
    String idm = messageId();
    String message = type + " " + idm + " " + content;
    listAdmin.add(idm);
    sendMessage(message);
  }


  public void sendAppMessage(String idApp, String content) {
    sendMessage("APPL", idApp + " " + content);
  }


  public void startMessageAdmin() {
    messAdminT = new Thread(messAdmin);
    messAdminT.start();
    jring.verbose.println("Message administrator started.");
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

