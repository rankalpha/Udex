#include <jni.h>
#include <string>
#include "jni/jni_help.h"
#include "util.h"
#include "dump_dex.h"
#include "runtime.h"

__attribute__((constructor)) static void init() {
    LOGV("init enter");
    runtime::InitRuntime();
    LOGV("init leave");
}

extern "C" JNIEXPORT void JNICALL
Java_abc_cookie_udex_DexHook_InitRuntime(
        JNIEnv* env,
        jclass clazz) {
    runtime::InitRuntime();
}

extern "C" JNIEXPORT jstring JNICALL
Java_abc_cookie_udex_DexHook_getPackageName(
        JNIEnv* env,
        jclass clazz) {
    std::string packageName = jni::GetPackageName(env);
    return env->NewStringUTF(packageName.c_str());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_abc_cookie_udex_DexHook_dumpDexBuffListByCookie(JNIEnv *env, jclass clazz,
                                                                 jlongArray cookie) {
    return dumpDexBuffListByCookie(env, clazz, cookie);
}

extern "C"
JNIEXPORT void JNICALL
Java_abc_cookie_udex_DexHook_dumpDexToLocalByCookie(JNIEnv *env, jclass clazz,
                                                                jlongArray cookie,
                                                                jstring dump_dir) {
    return dumpDexToLocalByCookie(env, clazz, cookie, dump_dir);
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_abc_cookie_udex_DexHook_getBaseDexClassLoaderList(JNIEnv *env, jclass clazz) {
    return getBaseDexClassLoaderList(env, clazz);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_abc_cookie_udex_DexHook_dumpMethodByMember(JNIEnv *env, jclass clazz,
                                                            jobject method) {
    return dumpMethodByMember(env, clazz, method);
}

extern "C"
JNIEXPORT void JNICALL
Java_abc_cookie_udex_DexHook_dumpMethods(JNIEnv *env, jclass clazz,
                                                 jobjectArray methods,
                                                 jobjectArray constructors,
                                                 jstring dump_dir) {
    return dumpMethods(env, clazz, methods, constructors, dump_dir);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_abc_cookie_udex_DexHook_dumpMethodByString(JNIEnv *env, jclass clazz, jclass cls,
                                                            jstring method_name,
                                                            jstring method_sign) {
    return dumpMethodByString(env, clazz, cls, method_name, method_sign);
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_abc_cookie_udex_DexHook_getClassLoaderList(JNIEnv *env, jclass clazz) {
    return getClassLoaderList(env, clazz);
}
