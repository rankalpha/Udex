package abc.cookie.udex;


import android.content.Context;
import android.util.Log;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import com.eclipsesource.json.Json;
import com.eclipsesource.json.JsonArray;
import com.eclipsesource.json.JsonObject;

import java.io.FileReader;
import java.io.IOException;

import dalvik.system.BaseDexClassLoader;
import dalvik.system.DexFile;

public class DexHook {
    static {
        //System.loadLibrary("udex");
    }

    public static final String TAG = "DEXHOOK";
    public static void PreLoadNativeSO(String libfile) {
        try {
            System.load(libfile);
        }catch (Exception e){
            Log.d(TAG, String.format("PreLoadNativeSo load %s error!", libfile));
        }
    }
    public static void dumpClass_old(String className){
        ClassLoader[] classLoaders =  getClassLoaderList();
        for (ClassLoader classLoader:classLoaders) {
            try {
                Class LoadEntry_cls =  classLoader.loadClass(className);
                Method[] Declaredmethods =  LoadEntry_cls.getDeclaredMethods();
                for (Method method :Declaredmethods ) {
                    dumpMethodByMember(method);
                }
                Constructor[] DeclaredConstructors =  LoadEntry_cls.getDeclaredConstructors();
                for (Constructor method :DeclaredConstructors ) {
                    dumpMethodByMember(method);
                }
                Log.d(TAG, String.format("dumpClass: %s",className));
            } catch (ClassNotFoundException e) {
            }
        }
    }
    public static void dumpDexToLocal_old(String sourceDir) {
        File pathFile = new File(sourceDir + "/dump");
        if(!pathFile.exists()){
            pathFile.mkdirs();
        }
        BaseDexClassLoader[] classLoaders = (BaseDexClassLoader[]) getBaseDexClassLoaderList();
        try {
            Log.d(TAG, "dumpDexToLocal: dump class start...");
            //TODO:to get 'pathList' field and 'dexElements' field by reflection.
            //private final DexPathList pathList;
            Class<?> baseDexClassLoaderClass = Class.forName("dalvik.system.BaseDexClassLoader");
            Field pathListField = baseDexClassLoaderClass.getDeclaredField("pathList");

            //private Element[] dexElements;
            Class<?> dexPathListClass = Class.forName("dalvik.system.DexPathList");
            Class<?> Element = Class.forName("dalvik.system.DexPathList$Element");
            Field dexElementsField = dexPathListClass.getDeclaredField("dexElements");
            Field DexFile_mCookie = DexFile.class.getDeclaredField("mCookie");
            Field DexFile_mFileName = DexFile.class.getDeclaredField("mFileName");
            Field path_field = Element.getDeclaredField("path");
            Field dexFile_field = Element.getDeclaredField("dexFile");
            pathListField.setAccessible(true);
            DexFile_mCookie.setAccessible(true);
            DexFile_mFileName.setAccessible(true);
            dexElementsField.setAccessible(true);
            dexFile_field.setAccessible(true);
            for (ClassLoader classLoader:classLoaders) {
                if (classLoader instanceof BaseDexClassLoader) {
                    Object BaseDexClassLoad_PathList = pathListField.get(classLoader);
                    Object[] DexPathList_dexElements = (Object[]) dexElementsField.get(BaseDexClassLoad_PathList);
                    int i = 0;
                    if (DexPathList_dexElements != null) {
                        for (Object dexElement : DexPathList_dexElements) {
                            DexFile dexFile = (DexFile) dexFile_field.get(dexElement);
                            if (dexFile != null) {
                                //这个cookie 在android 13是一个智能指针，保存的是一个native 的 DexFile 指针
                                long[] cookie = (long[]) DexFile_mCookie.get(dexFile);
                                dumpDexToLocalByCookie(cookie,pathFile.getAbsolutePath());
                                Log.d(TAG, String.format("dumpDexToLocalByCookie ok: hashCode: %x", dexFile.hashCode()));
                            }
                        }
                    }
                } else {
                    Log.d(TAG, "dump class not instanceof BaseDexClassLoader");
                }
            }
        } catch (Exception e) {
            Log.d(TAG, String.format("occue except: %s", e.toString()));
        }
    }
    public static void dumpDexToLocal(String sourceDir) {
        File pathFile=new File(sourceDir + "/dump");
        if(!pathFile.exists()){
            pathFile.mkdirs();
        }
        BaseDexClassLoader[] classLoaders = (BaseDexClassLoader[]) getBaseDexClassLoaderList();
        try {
            Log.d(TAG, "dumpDexToLocal enter");
            Runtime runtime = new Runtime();
            HashMap<long[], DexFile> cookies = runtime.findCookies(classLoaders);
            cookies.forEach((cookie, dexFile) -> {
                Log.d(TAG, String.format("dumpDexToLocalByCookie: dexFile: %x, cookie: %x", dexFile.hashCode(), cookie.hashCode()));
                dumpDexToLocalByCookie(cookie, pathFile.getAbsolutePath());
            });
            Log.d(TAG, "dumpDexToLocal leave");
        } catch (Exception e) {
            Log.d(TAG, String.format("occue except: %s", e.toString()));
        }
    }
    public static void dumpDexMethods(String sourceDir) {
        File pathFile = new File(sourceDir + "/dump");
        if(!pathFile.exists()){
            pathFile.mkdirs();
        }
        BaseDexClassLoader[] classLoaders = (BaseDexClassLoader[]) getBaseDexClassLoaderList();
        try {
            Log.d(TAG, "dumpDexMethods enter");
            Runtime runtime = new Runtime();
            HashMap<long[], Methods> allMethods = runtime.findMethodsByCookie(classLoaders);
            allMethods.forEach((cookie, methods) -> {
                Method[] meths = methods.GetMethods();
                Constructor[] ctors = methods.GetConstructors();
                Log.d(TAG, String.format("dumpMethods: cookie hashCode: %x, methods: %d, constructors: %d", cookie.hashCode(), meths.length, ctors.length));
                dumpMethods(meths, ctors, pathFile.getAbsolutePath());
            });
            Log.d(TAG, "dumpDexMethods leave");
        } catch (Exception e) {
            Log.d(TAG, String.format("occue except: %s", e.toString()));
        }
    }
    public static void dumpClassMethods(String sourceDir, String[] classNames) {
        File pathFile = new File(sourceDir + "/dump");
        if(!pathFile.exists()){
            pathFile.mkdirs();
        }
        BaseDexClassLoader[] classLoaders = (BaseDexClassLoader[]) getBaseDexClassLoaderList();
        try {
            Log.d(TAG, "dumpClassMethods enter");
            Runtime runtime = new Runtime();
            ArrayList<Methods> allMethods = runtime.findMethodsByClass(classLoaders, classNames);
            for (Methods methods : allMethods) {
                Method[] meths = methods.GetMethods();
                Constructor[] ctors = methods.GetConstructors();
                Log.d(TAG, String.format("dumpMethods: methods: %d, constructors: %d", meths.length, ctors.length));
                dumpMethods(meths, ctors, pathFile.getAbsolutePath());
            }
            Log.d(TAG, "dumpClassMethods leave");
        } catch (Exception e) {
            Log.d(TAG, String.format("occue except: %s", e.toString()));
        }
    }
    public static Config loadConfig(String jsonfile) {
        try {
            Config config = new Config();
            FileReader reader = new FileReader(jsonfile);
            JsonObject root = Json.parse(reader).asObject();
            config.isEnabled = root.get("isEnabled").asBoolean();
            config.isDumpDex = root.get("isDumpDex").asBoolean();
            config.isDumpCode = root.get("isDumpCode").asBoolean();
            config.isDumpCodeByClassList = root.get("isDumpCodeByClassList").asBoolean();
            config.delaySeconds = root.get("delaySeconds").asInt();
            JsonArray items = root.get("classList").asArray();
            int count = items.size();
            if (count > 0) {
                config.classList = new String[count];
                for (int i = 0; i < count; i ++) {
                    String className = items.get(i).asString();
                    config.classList[i] = className;
                }
            }
            Log.d(TAG, "------dump config enter-----");
            Log.d(TAG, config.toString());
            Log.d(TAG, "------dump config leave-----");
            return config;
        } catch (IOException e) {
            Log.d(TAG, String.format("loadConfig occue except: %s", e.toString()));
            return new Config();
        }
    }
    public static void startDumpTask(String packageName, Config config) {
        Log.d(TAG, "startDumpTask enter");
        String sourceDir = "/data/data/" + packageName;

        if (config.isDumpDex) {
            Log.d(TAG, "dumpdexToLocal enter");
            dumpDexToLocal(sourceDir);
            Log.d(TAG, "dumpdexToLocal leave");
        }
        if (config.isDumpCode) {
            Log.d(TAG, "dumpDexMethods enter");
            dumpDexMethods(sourceDir);
            Log.d(TAG, "dumpDexMethods leave");
        } else if (config.isDumpCodeByClassList) {
            Log.d(TAG, "dumpClassMethods enter");
            dumpClassMethods(sourceDir, config.classList);
            Log.d(TAG, "dumpClassMethods leave");
        }
        Log.d(TAG, "startDumpTask leave");
    }
    public static void startDumpTaskByThread(String packageName, Config config) {
        new Thread(){
            @Override
            public void run() {
                try {
                    Log.d(TAG, "startDumpTaskByThread enter");
                    Log.d(TAG, String.format("sleep seconds: %d", config.delaySeconds));
                    Thread.sleep(config.delaySeconds * 1000);
                    startDumpTask(packageName, config);
                    Log.d(TAG, "startDumpTaskByThread leave");
                } catch (Exception e) {
                    Log.d(TAG, String.format("startDumpTaskByThread occue except: %s", e.toString()));
                }
            }
        }.start();
    }
    public static void dumpPackage(String packageName) {
        Config config = loadConfig("/data/data/" + packageName + "/plugin/dump.json");
        if (config != null && config.isEnabled) {
            Log.d(TAG, String.format("config: %s", config.toString()));
            Log.d(TAG, "hookPackage " + packageName);
            if (config.delaySeconds > 0) {
                startDumpTaskByThread(packageName, config);
            } else {
                startDumpTask(packageName, config);
            }
        } else {
            Log.d(TAG, "pass " + packageName);
        }
    }
    public static String getProcessName() {
        try {
            Class activityThread_clazz = Class.forName("android.app.ActivityThread");
            Method method = activityThread_clazz.getDeclaredMethod("currentProcessName");
            method.setAccessible(true);
            return (String) method.invoke(null);
        } catch (Exception e) {
            Log.d(TAG, String.format("getProcessName occue except: %s", e.toString()));
            return "";
        }
    }
    public static void init() {
        Log.d(TAG, "init enter");
        String processName = getProcessName();
        Log.d(TAG, String.format("init leave: processName: %s", processName));
        if (processName.length() > 0) {
            String libfile = String.format("/data/data/%s/plugin/udex/libudex.so", processName);
            Log.d(TAG, String.format("PreLoadNativeSO enter: %s", libfile));
            PreLoadNativeSO(libfile);
            Log.d(TAG, "PreLoadNativeSO leave");
            String packageName = getPackageName();
            Log.d(TAG, String.format("packageName: %s", packageName));
            dumpPackage(packageName);
            Log.d(TAG, String.format("dumpPackage leave: %s", packageName));
        }
    }
    public static native void InitRuntime();
    public static native String getPackageName();
    public static native List<byte[]> dumpDexBuffListByCookie(long[] cookie);
    public static native void dumpDexToLocalByCookie(long[] cookie,String dumpDir);
    public static native ClassLoader[] getBaseDexClassLoaderList();
    public static native byte[] dumpMethodByMember(Member method);

    public static native void dumpMethods(Method[] methods, Constructor[] constructors, String dumpDir);
    public static native byte[] dumpMethodByString(Class<?> cls, String methodName, String methodSign);
    public static native ClassLoader[] getClassLoaderList();

}
