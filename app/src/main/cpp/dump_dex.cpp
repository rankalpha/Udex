#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>

void dumpMethodByMember(JNIEnv * env, jobject method) {

}

jobjectArray getClassLoadersList(JNIEnv * env) {

}

void dumpMethodsByMember(JNIEnv * env, jclass clazz) {
    // Get declared methods
    jmethodID getDeclaredMethodsMethod = env->GetMethodID(clazz, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
    jobjectArray declaredMethods = (jobjectArray)env->CallObjectMethod(clazz, getDeclaredMethodsMethod);
    jsize methods_count = env->GetArrayLength(declaredMethods);

    // Iterate over methods
    for (int j = 0; j < methods_count; j++) {
        jobject method = env->GetObjectArrayElement(declaredMethods, j);
        // Call dumpMethodByMember for each method
        dumpMethodByMember(env, method);
        // Release local reference
        env->DeleteLocalRef(method);
    }

    env->DeleteLocalRef(declaredMethods);
}

void dumpConstructorsByMember(JNIEnv * env, jclass clazz) {
    // Get declared constructors
    jmethodID getDeclaredConstructorsMethod = env->GetMethodID(clazz, "getDeclaredConstructors", "()[Ljava/lang/reflect/Constructor;");
    jobjectArray declaredConstructors = (jobjectArray)env->CallObjectMethod(clazz, getDeclaredConstructorsMethod);
    jsize constructors_count = env->GetArrayLength(declaredConstructors);

    // Iterate over constructors
    for (int j = 0; j < constructors_count; j++) {
        jobject constructor = env->GetObjectArrayElement(declaredConstructors, j);
        // Call dumpMethodByMember for each constructor
        dumpMethodByMember(env, constructor);
        // Release local reference
        env->DeleteLocalRef(constructor);
    }

    env->DeleteLocalRef(declaredConstructors);
}

jclass loadClass(JNIEnv * env, jobject classLoader, jstring className) {
    jclass classLoader_cls = env->GetObjectClass(classLoader);
    if (classLoader_cls == NULL) {
        return NULL;
    }
    jmethodID loadClassMethod = env->GetMethodID(classLoader_cls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if (loadClassMethod == NULL) {
        return NULL;
    }
    return (jclass)env->CallObjectMethod(classLoader, loadClassMethod, className);
}

void dumpClass(JNIEnv * env, jstring className) {
    jobjectArray classLoaders = getClassLoadersList(env);
    jsize classLoaderCount = env->GetArrayLength(classLoaders);

    env->ExceptionClear(); // Clear any pending exceptions
    for (int i = 0; i < classLoaderCount; i++) {
        jobject classLoader = env->GetObjectArrayElement(classLoaders, i);
        jclass clazz = loadClass(env, classLoader, className);
        if (clazz != NULL) {
            dumpMethodsByMember(env, clazz);
            dumpConstructorsByMember(env, clazz);
        }
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(classLoader);
    }
    if (env->ExceptionCheck()) {
        // Handle exception
    }
}

/*
void dumpInfo(JNIEnv * env, jobject dexElement, jfieldID pathField) {
    jstring pathStr = env->GetObjectField(dexElement, pathField);
    const char *pathChars = env->GetStringUTFChars(pathStr, NULL);
    __android_log_print(ANDROID_LOG_DEBUG, "NativeLibrary", "Dumping dex file: %s", pathChars);
    env->ReleaseStringUTFChars(pathStr, pathChars);
}

void dumpDexElements(JNIEnv * env, jobjectArray dexElements, char * path, jfieldID dexFileField, jfieldID mCookieField, jfieldID pathField) {
    jsize dexElementsCount = env->GetArrayLength(dexElements);
    for (int j = 0; j < dexElementsCount; j++) {
        jobject dexElement = env->GetObjectArrayElement(dexElements, j);
        jobject dexFile = env->GetObjectField(dexElement, dexFileField);
        if (dexFile == NULL) {
            continue;
        }
        jlong cookie = env->GetLongField(dexFile, mCookieField);
        // Call native method to dump dex to local
        dumpDexToLocalByCookie(cookie, path);
        dumpInfo(env, dexElement, pathField);
        env->DeleteLocalRef(dexElement);
    }
}

void dumpDexToLocal(JNIEnv *env, jclass clazz, jstring sourceDir) {
    const char *source_dir = env->GetStringUTFChars(sourceDir, NULL);
    if (source_dir == NULL) {
        return;
    }

    // Create directory if not exists
    const char * dump_dir = "/dump";
    char *path = (char *)malloc(strlen(source_dir) + strlen(dump_dir) + 1);
    strcpy(path, source_dir);
    strcat(path, dump_dir);
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // Get Field ID
    jclass baseDexClassLoaderClass = env->FindClass("dalvik/system/BaseDexClassLoader");
    jfieldID pathListField = env->GetFieldID(baseDexClassLoaderClass, "pathList", "Ldalvik/system/DexPathList;");
    jclass dexPathListClass = env->FindClass("dalvik/system/DexPathList");
    jfieldID dexElementsField = env->GetFieldID(dexPathListClass, "dexElements", "[Ldalvik/system/DexPathList$Element;");
    jclass dexElementClass = env->FindClass("dalvik/system/DexPathList$Element");
    jfieldID pathField = env->GetFieldID(dexElementClass, "path", "Ljava/lang/String;");
    jfieldID dexFileField = env->GetFieldID(dexElementClass, "dexFile", "Ldalvik/system/DexFile;");
    jclass dexFileClass = env->FindClass("dalvik/system/DexFile");
    jfieldID mCookieField = env->GetFieldID(dexFileClass, "mCookie", "J");
    //jfieldID mFileNameField = env->GetFieldID(dexFileClass, "mFileName", "?");

    // Set field visible
    env->ExceptionClear(); // Clear any pending exceptions
    env->SetFieldID(baseDexClassLoaderClass, "pathList", "Ldalvik/system/DexPathList;", JNI_TRUE);
    env->SetFieldID(dexPathListClass, "dexElements", "[Ldalvik/system/DexPathList$Element;", JNI_TRUE);
    env->SetFieldID(dexElementClass, "path", "Ljava/lang/String;", JNI_TRUE);
    env->SetFieldID(dexElementClass, "dexFile", "Ldalvik/system/DexFile;", JNI_TRUE);
    env->SetFieldID(dexFileClass, "mCookie", "J", JNI_TRUE);
    if (env->ExceptionCheck()) {
        // Handle exception
    }

    // Get BaseDexClassLoader list
    jobjectArray classLoaders = getBaseDexClassLoaderList(env);
    jsize classLoaderCount = env->GetArrayLength(classLoaders);
    for (int i = 0; i < classLoaderCount; i++) {
        jobject classLoader = env->GetObjectArrayElement(classLoaders, i);
        jobject pathList = env->GetObjectField(classLoader, pathListField);
        jobjectArray dexElements = env->GetObjectField(pathList, dexElementsField);
        if (dexElements == NULL) {
            continue;
        }
        dumpDexElements(env, dexElements, path, dexFileField, mCookieField, pathField);
        env->DeleteLocalRef(classLoader);
    }

    free(path);
    env->ReleaseStringUTFChars(sourceDir, source_dir);
}
*/