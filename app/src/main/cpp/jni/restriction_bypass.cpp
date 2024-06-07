#include <jni.h>
#include <android/log.h>
#include <thread>
#include <future>
#include <dlfcn.h>
#include "../base/scoped_fake_dlopen.h"
#include "../dlfcn/fake_dlfcn.h"
#include "../bypass_dl/bypass_dlopen.h"
#include "../Dobby/include/dobby.h"
#include "../util.h"

namespace jni {
// bool art::hiddenapi::ShouldDenyAccessToMember<art::ArtMethod>(art::ArtMethod *, std::function<art::hiddenapi::AccessContext ()(void)> const&, art::hiddenapi::AccessMethod)
// bool ShouldDenyAccessToMember<ArtMethod>(ArtMethod *, const&, art::hiddenapi::AccessMethod)

    extern "C" typedef bool (*fun_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod)(void *, void *, void *);
    void * orig_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod = NULL;

    extern "C"
    bool new_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod(void * a1, void * a2, void * a3) {
        return false;
    }

// bool art::hiddenapi::ShouldDenyAccessToMember<art::ArtField>(art::ArtField *, std::function<art::hiddenapi::AccessContext ()(void)> const&, art::hiddenapi::AccessMethod)
// bool ShouldDenyAccessToMember<ArtField>(ArtField *, const&, art::hiddenapi::AccessMethod)
    void * orig_art_hiddenapi_ShouldDenyAccessToMember_ArtField = NULL;

    extern "C"
    bool new_art_hiddenapi_ShouldDenyAccessToMember_ArtField(void * a1, void * a2, void * a3) {
        return false;
    }

    void hook() {
        ScopedFakeDlopen handle{ "libart.so" };
        void * func_addr1 = fake_dlsym(handle.get(), "_ZN3art9hiddenapi24ShouldDenyAccessToMemberINS_9ArtMethodEEEbPT_RKNSt3__18functionIFNS0_13AccessContextEvEEENS0_12AccessMethodE");
        if (func_addr1 != nullptr) {
            LOGV("ShouldDenyAccessToMember_ArtMethod: %p", func_addr1);
            DobbyHook(func_addr1, (void *)&new_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod, (void **)&orig_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod);
        } else {
            LOGV("ShouldDenyAccessToMember_ArtMethod is nullptr");
        }
        void * func_addr2 = fake_dlsym(handle.get(), "_ZN3art9hiddenapi24ShouldDenyAccessToMemberINS_8ArtFieldEEEbPT_RKNSt3__18functionIFNS0_13AccessContextEvEEENS0_12AccessMethodE");
        if (func_addr2 != nullptr) {
            LOGV("ShouldDenyAccessToMember_ArtField: %p", func_addr2);
            DobbyHook(func_addr2, (void *)&new_art_hiddenapi_ShouldDenyAccessToMember_ArtField, (void **)&orig_art_hiddenapi_ShouldDenyAccessToMember_ArtField);
        } else {
            LOGV("ShouldDenyAccessToMember_ArtField is nullptr");
        }
    }

    void hook1() {
        void *art_so_handle = bypass_dlopen("libart.so", RTLD_NOW);
        if (art_so_handle != nullptr) {
            void *func_addr1 = dlsym(art_so_handle, "_ZN3art9hiddenapi24ShouldDenyAccessToMemberINS_9ArtMethodEEEbPT_RKNSt3__18functionIFNS0_13AccessContextEvEEENS0_12AccessMethodE");
            DobbyHook(func_addr1, (void *)&new_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod, (void **)&orig_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod);
            void *func_addr2 = dlsym(art_so_handle, "_ZN3art9hiddenapi24ShouldDenyAccessToMemberINS_8ArtFieldEEEbPT_RKNSt3__18functionIFNS0_13AccessContextEvEEENS0_12AccessMethodE");
            DobbyHook(func_addr2, (void *)&new_art_hiddenapi_ShouldDenyAccessToMember_ArtField, (void **)&orig_art_hiddenapi_ShouldDenyAccessToMember_ArtField);
        }
    }

    void hook2() {
        const char * libart_name = "/apex/com.android.art/lib64/libart.so";
        void * func_addr1 = (void *) DobbySymbolResolver(libart_name, "_ZN3art9hiddenapi24ShouldDenyAccessToMemberINS_9ArtMethodEEEbPT_RKNSt3__18functionIFNS0_13AccessContextEvEEENS0_12AccessMethodE");
        DobbyHook(func_addr1, (void *)&new_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod, (void **)&orig_art_hiddenapi_ShouldDenyAccessToMember_ArtMethod);
        void * func_addr2 = (void *) DobbySymbolResolver(libart_name, "_ZN3art9hiddenapi24ShouldDenyAccessToMemberINS_8ArtFieldEEEbPT_RKNSt3__18functionIFNS0_13AccessContextEvEEENS0_12AccessMethodE");
        DobbyHook(func_addr2, (void *)&new_art_hiddenapi_ShouldDenyAccessToMember_ArtField, (void **)&orig_art_hiddenapi_ShouldDenyAccessToMember_ArtField);
    }
}
