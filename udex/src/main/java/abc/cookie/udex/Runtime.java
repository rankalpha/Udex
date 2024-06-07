package abc.cookie.udex;

import android.util.Log;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;
import java.util.ArrayList;
import dalvik.system.BaseDexClassLoader;
import dalvik.system.DexFile;

public class Runtime {
    public static final String TAG = "DEXHOOK";
    private Field pathListField;
    private Field DexFile_mCookie;
    private Field DexFile_mFileName;
    private Field path_field;
    private Field dexElementsField;
    private Field dexFile_field;

    public Runtime() throws ClassNotFoundException, NoSuchFieldException {
        //TODO:to get 'pathList' field and 'dexElements' field by reflection.
        //private final DexPathList pathList;
        Class<?> baseDexClassLoaderClass = Class.forName("dalvik.system.BaseDexClassLoader");
        pathListField = baseDexClassLoaderClass.getDeclaredField("pathList");

        //private Element[] dexElements;
        Class<?> dexPathListClass = Class.forName("dalvik.system.DexPathList");
        Class<?> Element = Class.forName("dalvik.system.DexPathList$Element");
        dexElementsField = dexPathListClass.getDeclaredField("dexElements");
        DexFile_mCookie = DexFile.class.getDeclaredField("mCookie");
        DexFile_mFileName = DexFile.class.getDeclaredField("mFileName");
        path_field = Element.getDeclaredField("path");
        dexFile_field = Element.getDeclaredField("dexFile");
        pathListField.setAccessible(true);
        DexFile_mCookie.setAccessible(true);
        DexFile_mFileName.setAccessible(true);
        dexElementsField.setAccessible(true);
        dexFile_field.setAccessible(true);
    }

    public HashMap<long[], DexFile> findCookies(BaseDexClassLoader[] classLoaders) throws IllegalAccessException {
        HashMap<long[], DexFile> cookies = new HashMap<long[], DexFile>();
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
                            cookies.put(cookie, dexFile);
                        }
                    }
                }
            }
        }
        return cookies;
    }

    private Method findMethod(BaseDexClassLoader[] classLoaders, String className, String fieldName) throws ClassNotFoundException {
        Method method = null;
        for (ClassLoader classLoader : classLoaders) {
            if (classLoader.toString().indexOf("java.lang.BootClassLoader") == -1) {
                Class DexFileClazz = classLoader.loadClass(className);
                for (Method field : DexFileClazz.getDeclaredMethods()) {
                    if (field.getName().equals(fieldName)) {
                        method = field;
                        method.setAccessible(true);
                        break;
                    }
                }
            }
        }
        return method;
    }

    private Class loadClass(BaseDexClassLoader[] classLoaders, String className, ClassLoader[] prevClassLoader) {
        try {
            if (prevClassLoader[0] != null) {
                return prevClassLoader[0].loadClass(className);
            }
        } catch (ClassNotFoundException e) {
        }

        for (ClassLoader classLoader : classLoaders) {
            if (classLoader.toString().indexOf("java.lang.BootClassLoader") == -1) {
                try {
                    Class cls = classLoader.loadClass(className);
                    prevClassLoader[0] = classLoader;
                    return cls;
                } catch (ClassNotFoundException e) {
                }
            }
        }
        return null;
    }
    public HashMap<long[], Methods> findMethodsByCookie(BaseDexClassLoader[] classLoaders) throws IllegalAccessException, ClassNotFoundException, InvocationTargetException, NoSuchFieldException {
        HashMap<long[], Methods> methodMaps = new HashMap<long[], Methods>();
        HashMap<long[], DexFile> cookies = findCookies(classLoaders);
        Method getClassNameListMethod = findMethod(classLoaders, "dalvik.system.DexFile", "getClassNameList");
        ClassLoader[] prevClassLoader = new ClassLoader[]{null};
        for (Map.Entry<long[], DexFile> entry : cookies.entrySet()) {
            long[] cookie = entry.getKey();
            DexFile dexFile = entry.getValue();
            String[] classNames = (String[])getClassNameListMethod.invoke(dexFile, cookie);
            Log.d(TAG, String.format("find class count: %d, cookie: %x", classNames.length, cookie.hashCode()));
            Methods methods = new Methods();
            for (int i = 0; i < classNames.length; i ++) {
                if (i % 1000 == 0) {
                    Log.d(TAG, String.format("   i: %d, cookie: %x", i, cookie.hashCode()));
                }
                String className = classNames[i];
                try {
                    //Class LoadEntry_cls = classLoader.loadClass(className);
                    Class LoadEntry_cls = loadClass(classLoaders, className, prevClassLoader);
                    if (LoadEntry_cls != null) {
                        try {
                            Method[] Declaredmethods = LoadEntry_cls.getDeclaredMethods();
                            methods.AddMethods(Declaredmethods);
                        } catch (NoClassDefFoundError e) {
                            Log.d(TAG, String.format("%s class getDeclaredMethods occue NoClassDefFoundError except!", className));
                        }
                        try {
                            Constructor[] DeclaredConstructors = LoadEntry_cls.getDeclaredConstructors();
                            methods.AddConstructors(DeclaredConstructors);
                        } catch (NoClassDefFoundError e) {
                            Log.d(TAG, String.format("%s class getDeclaredConstructors occue NoClassDefFoundError except!", className));
                        }
                    }
                } catch (Exception e) {
                    Log.d(TAG, String.format("parse %s class occue except!", className));
                }
            }
            methodMaps.put(cookie, methods);
        }
        return methodMaps;
    }
    public ArrayList<Methods> findMethodsByClass(BaseDexClassLoader[] classLoaders, String[] classNames) throws IllegalAccessException, ClassNotFoundException, InvocationTargetException, NoSuchFieldException {
        ArrayList<Methods> allMethods = new ArrayList<Methods>();
        Method getClassNameListMethod = findMethod(classLoaders, "dalvik.system.DexFile", "getClassNameList");
        ClassLoader[] prevClassLoader = new ClassLoader[]{null};
        Log.d(TAG, String.format("class count: %d", classNames.length));
        Methods methods = new Methods();
        for (int i = 0; i < classNames.length; i ++) {
            if (i % 1000 == 0) {
                Log.d(TAG, String.format("   i: %d", i));
            }
            String className = classNames[i];
            try {
                Class LoadEntry_cls = loadClass(classLoaders, className, prevClassLoader);
                if (LoadEntry_cls != null) {
                    try {
                        Method[] Declaredmethods = LoadEntry_cls.getDeclaredMethods();
                        methods.AddMethods(Declaredmethods);
                    } catch (NoClassDefFoundError e) {
                        Log.d(TAG, String.format("%s class getDeclaredMethods occue NoClassDefFoundError except!", className));
                    }
                    try {
                        Constructor[] DeclaredConstructors = LoadEntry_cls.getDeclaredConstructors();
                        methods.AddConstructors(DeclaredConstructors);
                    } catch (NoClassDefFoundError e) {
                        Log.d(TAG, String.format("%s class getDeclaredConstructors occue NoClassDefFoundError except!", className));
                    }
                }
            } catch (Exception e) {
                Log.d(TAG, String.format("parse %s class occue except!", className));
            }
        }
        allMethods.add(methods);
        return allMethods;
    }
}
