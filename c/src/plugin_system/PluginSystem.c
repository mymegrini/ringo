#include <jni.h>
#include "PluginSystem.h"
#define JAVA_PLUGIN_SYSTEM
#include "plugin_system.h"


struct PluginManagerEnv {
  JNIEnv *env;
  jobject obj;
} env;



/*
 * Class:     PluginSystem
 * Method:    initPluginManager
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_PluginSystem_initPluginManager
  (JNIEnv *env, jobject this)
{
  plugin_manager_init(&plugin_manager);
  env.env = env;
  env.obj = obj;
}

/*
 * Class:     PluginSystem
 * Method:    loadPlugin
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_PluginSystem_loadPlugin
  (JNIEnv *env, jobject this, jstring plugdir, jstring plugname)
{
  const char *c_plugdir = (*env)->GetStringUTFChars(env, plugdir, NULL);
  const char *c_plugname = (*env)->GetStringUTFChars(env, plugname, NULL);
  jint r = loadplugin(&plugin_manager, c_plugdir, c_plugname);
  (*env)->ReleaseStringUTFChars(env, plugdir, c_plugdir);
  (*env)->ReleaseStringUTFChars(env, plugname, c_plugname);

  return r;
}

/*
 * Class:     PluginSystem
 * Method:    unloadPlugin
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_PluginSystem_unloadPlugin
  (JNIEnv *env, jobject this, jstring plugname)
{
  const char *c_plugname = (*env)->GetStringUTFChars(env, plugname, NULL);
  jint r = unloadplugin(&plugin_manager, c_plugdir, c_plugname);
  (*env)->ReleaseStringUTFChars(env, plugname, c_plugname);

  return r;
}

/*
 * Class:     PluginSystem
 * Method:    loadAllPlugins
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_PluginSystem_loadAllPlugins
  (JNIEnv *env, jobject this, jstring plugdir)
{
  const char *c_plugdir = (*env)->GetStringUTFChars(env, plugdir, NULL);
  jint r = load_all_plugins(&plugin_manager, c_plugdir, c_plugname);
  (*env)->ReleaseStringUTFChars(env, plugdir, c_plugdir);

  return r;
}

/*
 * Class:     PluginSystem
 * Method:    unloadAllPlugins
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_PluginSystem_unloadAllPlugins
  (JNIEnv *env, jobject this)
{
  return (jint) unloadAllPlugins();
}

/*
 * Class:     PluginSystem
 * Method:    execPluginCommand
 * Signature: ([Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_PluginSystem_execPluginCommand
  (JNIEnv *env, jobject this, jobjectArray args)
{
  int argc = env->GetArrayLength(args);
  const char **argv = malloc(argc *sizeof(char *));

  for (int i = 0; i < argc; ++i) {
    jstring string = (jstring) GetObjectArrayElement(env, args, i);
    argv[i] = (*env)->GetStringUTFChars(env, string, NULL);
  }

  jint r = exec_plugin_command(&plugin_manager, argc, argv);

  for (int i = 0; i < argc; ++i) {
    jstring string = (jstring) GetObjectArrayElement(env, args, i);
    (*env)->ReleaseStringUTFChars(env, argv[i], string);
  }
  free(argv);

  return r;
}

/*
 * Class:     PluginSystem
 * Method:    execPluginAction
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_PluginSystem_execPluginAction
  (JNIEnv *env, jobject this, jstring idapp, jstring message, 
   jstring content, jint lookupflag)
{
  const char *c_idapp = (*env)->GetStringUTFChars(env, idapp, NULL);
  const char *c_message = (*env)->GetStringUTFChars(env, message, NULL);
  const char *c_content = (*env)->GetStringUTFChars(env, content, NULL);
  jint r = exec_plugin_action(&plugin_manager, 
      c_idapp, c_message, c_content, (int) lookupflag);
  (*env)->ReleaseStringUTFChars(env, idapp, c_idapp);
  (*env)->ReleaseStringUTFChars(env, message, c_message);
  (*env)->ReleaseStringUTFChars(env, content, c_content);

  return r;
}

/*
 * Class:     PluginSystem
 * Method:    getMessage
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_PluginSystem_getMessage
  (JNIEnv *env, jobject this)
{
  const char *message = get_message();
  return env->NewStringUTF(message); // free ??
}

/*
 * Class:     PluginSystem
 * Method:    printPlugins
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_PluginSystem_printPlugins
  (JNIEnv *env, jobject this)
{
  show_plugins();
}

