#include <android/log.h>
#include "runtime.h"
#include "base/utils.h"
#include "base/scoped_fake_dlopen.h"
#include "util.h"

namespace runtime {

#define ArtMethod_PrettyMethod_Symbol "_ZN3art9ArtMethod12PrettyMethodEb"
#define ArtMethod_GetObsoleteDexCache_Symbol "_ZN3art9ArtMethod19GetObsoleteDexCacheILNS_17ReadBarrierOptionE1EEENS_6ObjPtrINS_6mirror8DexCacheEEEv"
#define ArtMethod_GetCodeItem_Symbol "_ZN3art9ArtMethod11GetCodeItemEv"
#define NterpGetCodeItem_Symbol "NterpGetCodeItem"


    fun_art_ArtMethod_PrettyMethod ArtMethod_PrettyMethod = nullptr;
    fun_art_ArtMethod_GetObsoleteDexCache ArtMethod_GetObsoleteDexCache = nullptr;
    fun_art_ArtMethod_GetCodeItem ArtMethod_GetCodeItem = nullptr;
    fun_NterpGetCodeItem NterpGetCodeItem = nullptr;

    static bool has_inited = false;

    void InitRuntime() {
        if (has_inited) {
            return;
        }
        has_inited = true;
        LOGV("InitRuntime enter!");
        ScopedFakeDlopen handle{ "libart.so" };

        ArtMethod_PrettyMethod = reinterpret_cast<fun_art_ArtMethod_PrettyMethod>(fake_dlsym(handle.get(), ArtMethod_PrettyMethod_Symbol));
        ArtMethod_GetObsoleteDexCache = reinterpret_cast<fun_art_ArtMethod_GetObsoleteDexCache>(fake_dlsym(handle.get(), ArtMethod_GetObsoleteDexCache_Symbol));
        ArtMethod_GetCodeItem = reinterpret_cast<fun_art_ArtMethod_GetCodeItem>(fake_dlsym(handle.get(), ArtMethod_GetCodeItem_Symbol));
        NterpGetCodeItem = reinterpret_cast<fun_NterpGetCodeItem>(fake_dlsym(handle.get(), NterpGetCodeItem_Symbol));
        LOGV("PrettyMethod: %p, GetObsoleteDexCache: %p, GetCodeItem: %p, NterpGetCodeItem: %p", ArtMethod_PrettyMethod, ArtMethod_GetObsoleteDexCache, ArtMethod_GetCodeItem, NterpGetCodeItem);
        LOGV("InitRuntime leave!");
    }
}