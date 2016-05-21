import java.math.*;
import java.util.*;
import java.net.*;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class Entity{
  public String ip_next;
  public String ip_next2;
  public String mdiff_ip;
  public String mdiff_ip2;
  public int    mdiff_port;
  public int    port_next2;
  public int    port_next;
  public int    mdiff_port2;
  ////////////////////////////////////////////////////////////////////////////////
  // Internal classes
  ////////////////////////////////////////////////////////////////////////////////

  public static class NextEntity {

    public InetSocketAddress entity;
    public InetSocketAddress mdiff;
    public String nextIp;
    public String mdiffIp;
    public MulticastSocket mdiffSock;

    public NextEntity(InetAddress entityAddr, int entityPort, InetAddress mdiffAddr, int mdiffPort) {
      this(new InetSocketAddress(entityAddr, entityPort),
          new InetSocketAddress(mdiffAddr,  mdiffPort));
    }

    public NextEntity(InetSocketAddress entityAddr, InetSocketAddress mdiffAddr) {
      entity  = entityAddr;
      mdiff   = mdiffAddr;
      nextIp  = Jring.ipProtocolFormat(entityAddr.getAddress().toString());
      mdiffIp = Jring.ipProtocolFormat(mdiffAddr.getAddress().toString());
      nextIp  = nextIp.substring(1, nextIp.length());
      mdiffIp = mdiffIp.substring(1, mdiffIp.length());
      try {
        mdiffSock = new MulticastSocket(mdiffAddr.getPort());
        mdiffSock.joinGroup(mdiffAddr.getAddress());
      } catch (Exception e) {
        System.out.println("Multicast Socket creation error.");
        System.out.println(e);
        e.printStackTrace();
        System.exit(0);
      }
    }

    public void setEntity(InetSocketAddress entityAddr) {
      entity = entityAddr;
      nextIp  = Jring.ipProtocolFormat(entityAddr.getAddress().toString());
    }

    public void closeMdiff() {

    }
  }
  ////////////////////////////////////////////////////////////////////////////////
  // Attributes
  ////////////////////////////////////////////////////////////////////////////////

  public String id;
  public String paddedId;
  public String ip;
  public int    udp;
  public int    tcp;

  private boolean onRing;
  private ArrayList<NextEntity>        nextEntity;
  private DatagramSocket               listenSock;
  private DatagramSocket               sendSock;
  private final ReentrantReadWriteLock rwlock;

  public final Jring                  jring;
  ////////////////////////////////////////////////////////////////////////////////
  // Constructors
  ////////////////////////////////////////////////////////////////////////////////

  public Entity(Jring _jring, String ide,int u,int t) throws Exception {
    onRing     = false;
    jring      = _jring;

    id         = ide;
    paddedId   = Jring.padString(id, 8);
    udp        = u;
    tcp        = t;
    ip         = Jring.ipProtocolFormat(InetAddress.getLocalHost().getHostAddress());

    nextEntity = new ArrayList<>();
    listenSock = new DatagramSocket(u);
    sendSock   = new DatagramSocket();

    rwlock     = new ReentrantReadWriteLock();
  }


  ////////////////////////////////////////////////////////////////////////////////
  // Methods
  ////////////////////////////////////////////////////////////////////////////////
  public DatagramSocket getListenSock() { return listenSock; }


  public ArrayList<NextEntity> getNextEntity() { return nextEntity; }


  public boolean isOnRing() { 
    boolean val;
    rwlock.readLock().lock(); // not useful but...
    val = onRing;
    rwlock.readLock().unlock();
    return val;
  }


  public int findRing(String networkAddress, int port) {
    readLock();
    int i;
    for (i = 0; i < nextEntity.size(); i++)
      if (networkAddress.equals(nextEntity.get(i).mdiffIp) && port == nextEntity.get(i).mdiff.getPort())
        break;
    readUnlock();
    return (i == nextEntity.size())? -1: i;
  }


  public int ringNumber() {
    return nextEntity.size();
  }


  public void writeLock() {
    rwlock.writeLock().lock();
  }


  public void writeUnlock() {
    rwlock.writeLock().unlock();
  }


  public void readLock() {
    rwlock.readLock().lock();
  }


  public void readUnlock() {
    rwlock.readLock().unlock();
  }


  public void sendPacket(NextEntity next, String message) {
    byte[] b = message.getBytes();
    DatagramPacket p = new DatagramPacket(b, 0, 512, next.entity);
    try {
      sendSock.send(p);
    } catch (Exception e) {
      jring.verbose.println("Error sending packet " + message + " to " + 
          next.entity.getAddress() + " port " + next.entity.getPort());
    }
  }


  public void sendPacketAll(String packet) {
    rwlock.readLock().lock();
    for (NextEntity next : nextEntity)
      sendPacket(next, packet);
    rwlock.readLock().unlock();
  }



  public void insertNextEntity(NextEntity next) {
    if (isOnRing()) {
      rwlock.writeLock().lock();
      nextEntity.set(nextEntity.size()-1, next);
      rwlock.writeLock().unlock();
    } else {
      addNextEntity(next);
    }
  }


  public void insertNextEntity(InetSocketAddress next) {
    rwlock.writeLock().lock();
    nextEntity.get(nextEntity.size()-1).entity = next;
    rwlock.writeLock().unlock();
  }


  public void addNextEntity(NextEntity next) {
    rwlock.writeLock().lock();
    nextEntity.add(next);
    rwlock.writeLock().unlock();
  }


  public void addNextEntity(InetSocketAddress nextAddr, InetSocketAddress mdiffAddr) {
    addNextEntity(new NextEntity(nextAddr, mdiffAddr));
  }



  public void all_info(){
    // System.out.println("Identity "+id+" Ip "+ip);
    // System.out.println("Port udp "+udp+" Port tcp "+tcp);
    // System.out.println("Next ip "+ip_next+" Next port "+port_next);
    // System.out.println("Diffusion ip "+mdiff_ip+" Diffusion port "+mdiff_port);
    // if(port_next2!=-1){
    //   System.out.println("Next ip "+ip_next2+" Next port "+port_next2);
    //   System.out.println("Diffusion ip "+mdiff_ip2+" Diffusion port "+mdiff_port2);
    // }
  }

  public static String ip_form(String ip){
    String []tab = ip.split("\\.");
    if(ip.length()==15) return ip;
    return add_zero(tab[0],3)+"."+add_zero(tab[1],3)+"."+add_zero(tab[2],3)+"."+add_zero(tab[3],3);
  }


  public static String ipFormat(String ip) {
    String f = "";
    return f;
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
