//
// Created by thehepta on 2023/8/9.
//

#include <android/log.h>
#include <jni.h>
#include <stdlib.h>
#include <sys/system_properties.h>
#include <string>
#include <vector>
#include "RuntimePart.h"
#include "runtime/class_linker.h"
#include "elf_symbol_resolver.h"

#define LOGV(...)  ((void)__android_log_print(ANDROID_LOG_INFO, "UDEX", __VA_ARGS__))

AndroidRunAPI *AndroidRunAPI::androidRunApi = nullptr;

template <typename T>
int findOffset(void *start, int regionStart, int regionEnd, T value) {
    if (NULL == start || regionEnd <= 0 || regionStart < 0) {
        return -1;
    }
    char *c_start = (char *)start;

    for (int i = regionStart; i < regionEnd; i += 4) {
        T *current_value = (T *)(c_start + i);
        if (value == *current_value) {
            //            LOGV("found offset: %d", i);
            return i;
        }
    }
    return -2;
}

int get_heap_to_jvm_offset() {
    int apiLevel = android_get_device_api_level();
    if (apiLevel > 34) {
        return sizeof(PartialRuntime14);
    } else if (apiLevel == 34) {
        return sizeof(PartialRuntime14);
    } else if (apiLevel == 33) {
        return sizeof(PartialRuntime13);
    } else if (apiLevel == 32) {
        return sizeof(PartialRuntime12_1);
    } else if (apiLevel == 31) {
        return sizeof(PartialRuntime12);
    } else if (apiLevel == 30) {
        return sizeof(PartialRuntime11);
    } else if (apiLevel == 29) {
        return sizeof(PartialRuntime10);
    } else if (apiLevel == 28) {
        return sizeof(PartialRuntime9);
    } else if (apiLevel == 27) {
        return sizeof(PartialRuntime8_1);
    } else if (apiLevel == 26) {
        return sizeof(PartialRuntime8);
    }
}

class LookupClassesVisitor : public ClassLoaderVisitor {
public:
    LookupClassesVisitor(JNIEnv *env, JavaVMExt *pExt) {
        int offsetOfVmExt = findOffset(env, 0, 10000, (void *)pExt);
        //        LOGV("g_env offsetOfVmExt = %d",offsetOfVmExt);
        uint8_t *u_env = reinterpret_cast<uint8_t *>(env);
        jniEnvExt = reinterpret_cast<JNIEnvExt *>(u_env + offsetOfVmExt - sizeof(void *));
    }

    void Visit(ObjPtr<Object> class_loader) REQUIRES_SHARED(Locks::classlinker_classes_lock_, Locks::mutator_lock_) OVERRIDE {
        AndroidRunAPI *androidRunApi = AndroidRunAPI::getInstance();
        jobject classLoader_obj = androidRunApi->AddGlobalRef(jniEnvExt->vm_, jniEnvExt->self_, class_loader);
        vec_obj.push_back(classLoader_obj);
    }

    std::vector<jobject> getVecObj() { return vec_obj; }

private:
    JNIEnvExt *jniEnvExt;
    std::vector<jobject> vec_obj;
};

void getAndroidSystemFunction() {
    AndroidRunAPI *androidRunApi = AndroidRunAPI::getInstance();

    androidRunApi->VisitClassLoaders =
            (void (*)(void *, void *))resolve_elf_internal_symbol("libart.so", "_ZNK3art11ClassLinker17VisitClassLoadersEPNS_18ClassLoaderVisitorE");
    androidRunApi->AddGlobalRef = (jobject(*)(void *, void *, ObjPtr<Object>))resolve_elf_internal_symbol(
            "libart.so", "_ZN3art9JavaVMExt12AddGlobalRefEPNS_6ThreadENS_6ObjPtrINS_6mirror6ObjectEEE");
}

jobjectArray getClassLoaders(JNIEnv *env, jint targetSdkVersion) {
    JavaVM *javaVM;
    env->GetJavaVM(&javaVM);
    JavaVMExt *javaVMExt = (JavaVMExt *)javaVM;
    void *runtime = javaVMExt->runtime;

    LOGV("runtime ptr: %p, vmExtPtr: %p", runtime, javaVMExt);
    LOGV("std::unique_ptr<PartialRuntime14> size: %d", sizeof(std::unique_ptr<PartialRuntime14>));
    LOGV("get_heap_to_jvm_offset size: %d", get_heap_to_jvm_offset());

    const int MAX = 5000;
    int offsetOfVmExt = findOffset(runtime, 0, MAX, (void *)javaVMExt);
    LOGV("offsetOfVmExt: %d", offsetOfVmExt);
    int head_offset = offsetOfVmExt - get_heap_to_jvm_offset() + sizeof(void *);
    LOGV("head_offset: %d", head_offset);
    void *heap = (char *)runtime + head_offset;
    AndroidRunAPI *androidRunApi = AndroidRunAPI::getInstance();
    androidRunApi->partialRuntime = static_cast<PartialRuntime *>(heap);
    getAndroidSystemFunction();
    LookupClassesVisitor visitor(env, javaVMExt);

    androidRunApi->VisitClassLoaders(androidRunApi->partialRuntime->class_linker_, &visitor);

    std::vector<jobject> vectorObject = visitor.getVecObj();
    jclass ClassLoader_cls = env->FindClass("java/lang/ClassLoader");

    for (auto it = vectorObject.begin(); it != vectorObject.end(); /* no increment here */) {
        jboolean re = env->IsInstanceOf(*it, ClassLoader_cls);
        if (!re) {
            it = vectorObject.erase(it);
        } else {
            ++it;
        }
    }

    jobjectArray objectArray = env->NewObjectArray(vectorObject.size(), ClassLoader_cls, NULL);

    for (int i = 0; i < vectorObject.size(); i++) {
        env->SetObjectArrayElement(objectArray, i, vectorObject[i]);
    }
    return objectArray;
}

jobjectArray getBaseDexClassLoaders(JNIEnv *env, jint targetSdkVersion) {
    JavaVM *javaVM;
    env->GetJavaVM(&javaVM);
    JavaVMExt *javaVMExt = (JavaVMExt *)javaVM;
    void *runtime = javaVMExt->runtime;

    LOGV("runtime ptr: %p, vmExtPtr: %p", runtime, javaVMExt);
    LOGV("std::unique_ptr<PartialRuntime13> size: %d", sizeof(std::unique_ptr<PartialRuntime13>));
    LOGV("get_heap_to_jvm_offset size: %d", get_heap_to_jvm_offset());

    const int MAX = 5000;
    int offsetOfVmExt = findOffset(runtime, 0, MAX, (void *)javaVMExt);
    LOGV("offsetOfVmExt: %d", offsetOfVmExt);
    int head_offset = offsetOfVmExt - get_heap_to_jvm_offset() + sizeof(void *);
    LOGV("head_offset: %d", head_offset);
    void *heap = (char *)runtime + head_offset;
    AndroidRunAPI *androidRunApi = AndroidRunAPI::getInstance();
    androidRunApi->partialRuntime = static_cast<PartialRuntime *>(heap);
    getAndroidSystemFunction();
    LookupClassesVisitor visitor(env, javaVMExt);

    androidRunApi->VisitClassLoaders(androidRunApi->partialRuntime->class_linker_, &visitor);

    std::vector<jobject> vectorObject = visitor.getVecObj();
    jclass ClassLoader_cls = env->FindClass("dalvik/system/BaseDexClassLoader");

    for (auto it = vectorObject.begin(); it != vectorObject.end(); /* no increment here */) {
        jboolean re = env->IsInstanceOf(*it, ClassLoader_cls);
        if (!re) {
            it = vectorObject.erase(it);
        } else {
            ++it;
        }
    }

    jobjectArray objectArray = env->NewObjectArray(vectorObject.size(), ClassLoader_cls, NULL);

    for (int i = 0; i < vectorObject.size(); i++) {
        env->SetObjectArrayElement(objectArray, i, vectorObject[i]);
    }
    return objectArray;
}
