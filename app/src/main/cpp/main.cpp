#include <jni.h>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <thread>
#include <dlfcn.h>

#include "util.h"
#include "runtime.h"
#include "runtime/dex_file.h"
#include "runtime/art_method.h"
#include "runtime/dex_file_structs.h"
#include "Dobby/include/dobby.h"
#include "base/file_utils.h"
#include "base/utils.h"
#include "base/scoped_fake_dlopen.h"
#include "jni/jni_help.h"
#include "jni/restriction_bypass.h"

#define XPOSED_LOADER_ENTRY_CLASS_NAME            "abc.cookie.udex.DexHook"
#define XPOSED_LOADER_ENTRY_METHOD_NAME           "init"
#define ART_HOOK_LIBRARY_SO_NAME                  "libudex.so"

#define TEMP_ART_HOOK_LIBRARY_SO_FILE_PATH        "/data/local/tmp/plugin/udex/libudex.so"
#define TEMP_ART_HOOK_LIBRARY_SO_FILE_PATH_64     "/data/local/tmp/plugin/udex/libudex.so"

static void LoadXposedModules(JNIEnv* env) {
    jni::CallStaticMethodByJavaMethodInvoke(env, XPOSED_LOADER_ENTRY_CLASS_NAME, XPOSED_LOADER_ENTRY_METHOD_NAME);
}

static bool sHasInjectedSuccess = false;

static void InjectXposedLibraryInternal(JNIEnv* env) {
    LOGD("Start Inject Xposed Library.");
    if (sHasInjectedSuccess) return;
    sHasInjectedSuccess = true;

    std::string package_name1 = jni::GetPackageName(env);
    LOGD("env: %x, package_name: %s", env, package_name1.c_str());

    int api_level = poros::GetDeviceApiLevel();
    if (api_level >= 28) {  // android P
        jni::hook();
        //jni::BypassHiddenApiByRefection(env);
    }

    std::string package_name = jni::GetPackageName(env);
    LOGD("Android Debug Injection, start insert dex and so into the application: %s", package_name.c_str());

    //auto udex_path = "/data/data/" + package_name + "/plugin/udex";
    //poros::mk_all_dir(udex_path.c_str());

    auto dex_path = "/data/data/" + package_name + "/plugin/udex/classes.dex";
    //const char* temp_dex_file_path = "/data/local/tmp/plugin/udex/classes.dex";
    //poros::copy_file(temp_dex_file_path, dex_path.c_str());

    //std::string so_path;
    //const char* temp_hook_so_path;
    //if (poros::Is64BitRuntime()) {
    //    so_path = "/data/data/" + package_name + "/plugin/udex";
    //    temp_hook_so_path = TEMP_ART_HOOK_LIBRARY_SO_FILE_PATH_64;
    //}
    //else {
    //    so_path = "/data/data/" + package_name + "/plugin/udex";
    //    temp_hook_so_path = TEMP_ART_HOOK_LIBRARY_SO_FILE_PATH;
    //}

    auto so_path = "/data/data/" + package_name + "/plugin/udex";
    //std::string dst_hook_so_path = "/data/data/" + package_name + "/plugin/udex/" + ART_HOOK_LIBRARY_SO_NAME;
    //poros::mk_all_dir(so_path.c_str());

    //poros::copy_file(temp_hook_so_path, dst_hook_so_path.c_str());

    //auto target_plugin_apk_dir = "/data/data/" + package_name + "/plugin/udex";
    //poros::mk_all_dir(target_plugin_apk_dir.c_str());

    //auto target_plugin_apk_path = target_plugin_apk_dir + "/plugin.apk";
    //const char* temp_plugin_apk_path = "/data/local/tmp/plugin/plugin.apk";
    //poros::copy_file(temp_plugin_apk_path, target_plugin_apk_path.c_str());

    //auto target_config_dir = "/data/data/" + package_name + "/plugin";
    //poros::mk_all_dir(target_config_dir.c_str());

    //auto target_config_path = "/data/data/" + package_name + "/plugin/dump.json";
    //const char* temp_config_path = "/data/local/tmp/plugin/dump.json";
    //poros::copy_file(temp_config_path, target_config_path.c_str());

    jni::MergeDexAndSoToClassLoader(env, dex_path.c_str(), so_path.c_str());

    LoadXposedModules(env);
}

// deprecated, inject Xposed module on another thread.
static void InjectXposedLibraryAsync(JNIEnv* jni_env) {
    LOGD(" Inject Xposed Library from work thread.");
    JavaVM* javaVm;
    jni_env->GetJavaVM(&javaVm);
    std::thread worker([javaVm]() {
        JNIEnv* env;
        javaVm->AttachCurrentThread(&env, nullptr);

        int count = 0;
        while (true && count < 10000) {
            jobject app_loaded_apk_obj = jni::GetLoadedApkObj(env);
            LOGD(" loadedApk obj is -> %p wait count -> %d", app_loaded_apk_obj, count);
            // wait here until loaded apk object is available
            if (app_loaded_apk_obj == nullptr) {
                usleep(100);
            }
            else {
                break;
            }
            count++;
        }

        LOGD("start usleep ...");
        //usleep(10 * 1000 * 1000);
        LOGD("end usleep ...");
        InjectXposedLibraryInternal(env);

        if (env) {
            javaVm->DetachCurrentThread();
        }
    });
    worker.detach();
}

extern "C" typedef void (*fun_art_ArtMethod_Invoke)(void *, void *, uint32_t *, uint32_t, void *, const char *);
void * orig_art_ArtMethod_Invoke = NULL;
void handle_inject();

extern "C"
void new_art_ArtMethod_Invoke(void *thiz, void *thread, uint32_t *args, uint32_t args_size, void *result, const char *shorty) {
    ((fun_art_ArtMethod_Invoke)orig_art_ArtMethod_Invoke)(thiz, thread, args, args_size, result, shorty);
    handle_inject();
}

void hook_ArtMethod_Invoke() {
    const char *libart_name = "/apex/com.android.art/lib64/libart.so";
    void * func_addr = (void *) DobbySymbolResolver(libart_name, "_ZN3art9ArtMethod6InvokeEPNS_6ThreadEPjjPNS_6JValueEPKc");
    DobbyHook(func_addr, (void *)&new_art_ArtMethod_Invoke, (void **)&orig_art_ArtMethod_Invoke);
}

extern "C" typedef int (*fun_art_interpreter_EnterInterpreterFromInvoke)(void *, void *, uint32_t *, uint32_t *, int64_t *, int);
void * orig_art_interpreter_EnterInterpreterFromInvoke = NULL;

extern "C"
int new_art_interpreter_EnterInterpreterFromInvoke(void * thread, void * method, uint32_t * receiver, uint32_t * args, int64_t * result, int stay_in_interpreter) {
    //LOGV("art_interpreter_EnterInterpreterFromInvoke(6) enter");
    int ret = ((fun_art_interpreter_EnterInterpreterFromInvoke)orig_art_interpreter_EnterInterpreterFromInvoke)(thread, method, receiver, args, result, stay_in_interpreter);
    //LOGV("art_interpreter_EnterInterpreterFromInvoke(6) leave");
    handle_inject();
    return ret;
}

void hook_art_interpreter_EnterInterpreterFromInvoke() {
    const char *libart_name = "/apex/com.android.art/lib64/libart.so";
    void * func_addr = (void *) DobbySymbolResolver(libart_name, "_ZN3art11interpreter26EnterInterpreterFromInvokeEPNS_6ThreadEPNS_9ArtMethodENS_6ObjPtrINS_6mirror6ObjectEEEPjPNS_6JValueEb");
    DobbyHook(func_addr, (void *)&new_art_interpreter_EnterInterpreterFromInvoke, (void **)&orig_art_interpreter_EnterInterpreterFromInvoke);
}

static bool has_injected = false;
void handle_inject() {
    if (!has_injected) {
        void* current_thread_ptr = runtime::CurrentThreadFunc();
        if (current_thread_ptr == nullptr) {
            return ;
        }
        LOGV("runtime::CurrentThreadFunc: %p", current_thread_ptr);
        JNIEnv *env = runtime::GetJNIEnvFromThread(current_thread_ptr);
        LOGV("runtime::GetJNIEnvFromThread: env: %p", env);
        has_injected = true;
        InjectXposedLibraryAsync(env);
    }
}

static bool has_hooked = false;
void DoInject() {
    void* current_thread_ptr = runtime::CurrentThreadFunc();
    if (current_thread_ptr == nullptr) {
        if (!has_hooked) {
            has_hooked = true;
            LOGV("current thread ptr is nullptr, start hook_ArtMethod_Invoke ...");
            hook_ArtMethod_Invoke();
            //hook_art_interpreter_EnterInterpreterFromInvoke();
        } else {
            LOGV("BUG!!! always hook_ArtMethod_Invoke! but current thread ptr is nullptr!");
        }
    } else {
        handle_inject();
    }
}

__attribute__((constructor)) static void init() {
    LOGV("init enter");
    runtime::InitRuntime();
    //DoInject();
    hook_ArtMethod_Invoke();
    LOGV("init leave");
}
