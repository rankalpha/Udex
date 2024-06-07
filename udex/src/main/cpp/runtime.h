#pragma once

#include <jni.h>
#include <string>

namespace runtime {
    typedef std::string (*fun_art_ArtMethod_PrettyMethod)(void *, bool);
    typedef uint64_t (*fun_art_ArtMethod_GetObsoleteDexCache)(void *);
    typedef void * (*fun_NterpGetCodeItem)(void *);

    extern fun_art_ArtMethod_PrettyMethod ArtMethod_PrettyMethod;
    extern fun_art_ArtMethod_GetObsoleteDexCache ArtMethod_GetObsoleteDexCache;
    extern fun_NterpGetCodeItem NterpGetCodeItem;

    void InitRuntime();
}