public class PluginSystem {
  static {
    System.loadLibrary("plugin_system_java.so");
  }

  public native void initPluginManager();

  public native int loadPlugin(String plugdir, String plugname);

  public native int unloadPlugin(String plugname);

  public native int loadAllPlugins(String plugdir);

  public native int unloadAllPlugins();

  public native int execPluginCommand(String args[]);

  public native int execPluginAction(String idapp, String message,
      String content, int lookupflag);

  public native String getMessage();

  public native void printPlugins();

  private Entity ent;

  public PluginSystem(Entity _ent) {
    ent = _ent;
    initPluginManager();
  }
}
