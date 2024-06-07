#include <android/log.h>
#include <fcntl.h>
#include <jni.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>

#include "RuntimePart.h"
#include "art_method.h"
#include "runtime/dex_file.h"
#include "runtime.h"
#include "util.h"

inline static bool IsIndexId(jmethodID mid) { return ((reinterpret_cast<uintptr_t>(mid) % 2) != 0); }

jobjectArray getClassLoaderList(JNIEnv *env, jclass thiz) { return getClassLoaders(env, android_get_application_target_sdk_version()); }
jobjectArray getBaseDexClassLoaderList(JNIEnv *env, jclass thiz) { return getBaseDexClassLoaders(env, android_get_application_target_sdk_version()); }

static jfieldID field_art_method = nullptr;
//CodeItem *(*GetCodeItem_fun)(void *this_ptr);
std::map<std::string, const DexFile *> dex_fd_maps;

template <typename Dest, typename Source>
inline Dest reinterpret_cast64(Source source) {
    return reinterpret_cast<Dest>(static_cast<uintptr_t>(source));
}

constexpr size_t kOatFileIndex = 0;
constexpr size_t kDexFileIndexStart = 1;
static bool ConvertJavaArrayToDexFiles(JNIEnv *env, jobject cookie_arrayObject,
        /*out*/ std::vector<const DexFile *> &dex_files,
        /*out*/ const OatFile *&oat_file) {
    jarray array = reinterpret_cast<jarray>(cookie_arrayObject);

    jsize array_size = env->GetArrayLength(array);
    if (env->ExceptionCheck() == JNI_TRUE) {
        return false;
    }

    // TODO: Optimize. On 32bit we can use an int array.
    jboolean is_long_data_copied;
    jlong *long_data = env->GetLongArrayElements(reinterpret_cast<jlongArray>(cookie_arrayObject), &is_long_data_copied);
    if (env->ExceptionCheck() == JNI_TRUE) {
        return false;
    }

    oat_file = reinterpret_cast64<const OatFile *>(long_data[kOatFileIndex]);
    dex_files.reserve(array_size - 1);
    for (jsize i = kDexFileIndexStart; i < array_size; ++i) {
        dex_files.push_back(reinterpret_cast64<const DexFile *>(long_data[i]));
    }
    // b4000079b3af9740
    env->ReleaseLongArrayElements(reinterpret_cast<jlongArray>(array), long_data, JNI_ABORT);
    return env->ExceptionCheck() != JNI_TRUE;
}

jobject dumpDexBuffListByCookie(JNIEnv *env, jclass clazz, jlongArray cookie) {
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID arrayListInit = env->GetMethodID(arrayListClass, "<init>", "()V");
    jmethodID arrayListAdd = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
    // 创建一个 ArrayList 实例
    jobject arrayList = env->NewObject(arrayListClass, arrayListInit);
    // 获取 byte[] 类和构造函数
    std::vector<const DexFile *> dex_files;
    const OatFile *oat_file;
    bool re = ConvertJavaArrayToDexFiles(env, cookie, dex_files, oat_file);
    if (!re) {
        LOGV("dumpDexByCookie ConvertJavaArrayToDexFiles failed\n");
        return arrayList;
    }
    for (int i = 0; i < dex_files.size(); i++) {
        const DexFile *dex_file = dex_files[i];
        if (dex_file != nullptr) {
            jbyteArray byteArray = env->NewByteArray(dex_file->size_);
            env->SetByteArrayRegion(byteArray, 0, dex_file->size_, reinterpret_cast<const jbyte *>(dex_file->begin_));
            env->CallBooleanMethod(arrayList, arrayListAdd, byteArray);
            env->DeleteLocalRef(byteArray);
        }
    }
    return arrayList;
}

void save_dex_file(std::string dumpDir, const DexFile * dex_file) {
    char dumpdex_path[100] = {0};
    sprintf(dumpdex_path, (dumpDir + "/%p.dex").c_str(), dex_file->begin_);
    LOGV("dumpdex_path = %s", dumpdex_path);

    std::string dex_filepath(dumpdex_path);
    if (dex_fd_maps.find(dex_filepath) != dex_fd_maps.end()) {
        return ;
    }

    int dumpdex_fd = open(dumpdex_path, O_CREAT | O_RDWR, 0644);
    if (dumpdex_fd < 0) {
        LOGV("Error: opening %s file failed!", dumpdex_path);
        return;
    }
    size_t bytes_written = write(dumpdex_fd, dex_file->begin_, dex_file->size_);  // 将内存中的数据写入文件
    if (bytes_written != dex_file->size_) {
        // LOGV("Error writing to file.\n");
    } else {
        // LOGV("Data successfully written to file:%s.\n",dumpdex_path);
    }
    close(dumpdex_fd);
    dex_fd_maps.emplace(dex_filepath, dex_file);
}

void dumpDexToLocalByCookie(JNIEnv *env, jclass thiz, jlongArray cookie, jstring jdumpDir) {
    LOGV("dumpDexToLocalByCookie enter");
    std::vector<const DexFile *> dex_files;
    std::string dumpDir = env->GetStringUTFChars(jdumpDir, nullptr);
    const OatFile *oat_file;
    bool ret = ConvertJavaArrayToDexFiles(env, cookie, dex_files, oat_file);
    for (int i = 0; i < dex_files.size(); i++) {
        const DexFile *dex_file = dex_files[i];
        if (dex_file != nullptr) {
            save_dex_file(dumpDir, dex_file);
        }
    }
    LOGV("dumpDexToLocalByCookie leave");
    return;
}

// Executable 的artMethod 可能是隐藏的，需要注意是否能获取
jfieldID getArtMethod_field(JNIEnv *env) {
    jclass clazz = env->FindClass("java/lang/reflect/Executable");
    auto field = env->GetFieldID(static_cast<jclass>(clazz), "artMethod", "J");
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return field;
}

//传入的是method索引
ArtMethod *GetArtMethod(JNIEnv *env, jclass clazz, jmethodID methodId) {
    if (android_get_device_api_level() >= __ANDROID_API_R__) {
        if (IsIndexId(methodId)) {
            jobject method = env->ToReflectedMethod(clazz, methodId, true);
            return reinterpret_cast<ArtMethod *>(env->GetLongField(method, field_art_method));
        }
    }
    return reinterpret_cast<ArtMethod *>(methodId);
}

//传入的不是method索引，而是method对象的索引
ArtMethod *GetArtMethod(JNIEnv *env, jclass clazz, jobject methodId) {
    if (android_get_device_api_level() >= __ANDROID_API_R__) {
        jfieldID field_art_method = getArtMethod_field(env);
        return reinterpret_cast<ArtMethod *>(env->GetLongField(methodId, field_art_method));
    }
    return reinterpret_cast<ArtMethod *>(methodId);
}

bool WriteCodeItemToDexFileByArtMethod(ArtMethod *artMethod, uint32_t code_item_len) {
    uint8_t *code_item = reinterpret_cast<uint8_t *>(ArtMethod::GetCodeItem(artMethod));
    uintptr_t code_item_addr = reinterpret_cast<uintptr_t>(code_item);
    for (auto dex_fd : dex_fd_maps) {
        std::string dumpdex_path = dex_fd.first;
        const DexFile *dexFile = dex_fd.second;
        uintptr_t dex_begin = (uintptr_t)dexFile->begin_;
        uintptr_t dex_end = dex_begin + dexFile->size_;
        if (dex_begin < code_item_addr && code_item_addr < dex_end) {
            uintptr_t writ_off = code_item_addr - dex_begin;
            int dumpdex_fd = open(dumpdex_path.c_str(), O_RDWR, 0644);
            lseek(dumpdex_fd, writ_off, SEEK_SET);
            write(dumpdex_fd, code_item, code_item_len);
            close(dumpdex_fd);
            LOGV("fix code_item successful dumpdex_path = %s code_item_len = %d  writ_off = %d ", dumpdex_path.c_str(), code_item_len, writ_off);
            return true;
        }
    }
    return false;
}

extern "C" char * base64_encode(char *str,long str_len,long* outlen){
    long len;
    char *res;
    int i,j;
    const char *base64_table="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if(str_len % 3 == 0) {
        len = str_len / 3 * 4;
    } else {
        len = (str_len / 3 + 1) * 4;
    }
    res=(char*)malloc(sizeof(char)*(len+1));
    res[len]='\0';
    *outlen=len;
    for(i=0,j=0;i<len-2;j+=3,i+=4) {
        res[i]=base64_table[str[j]>>2];
        res[i+1]=base64_table[(str[j]&0x3)<<4 | (str[j+1]>>4)];
        res[i+2]=base64_table[(str[j+1]&0xf)<<2 | (str[j+2]>>6)];
        res[i+3]=base64_table[str[j+2]&0x3f];
    }
    switch(str_len % 3) {
    case 1:
        res[i-2]='=';
        res[i-1]='=';
        break;
    case 2:
        res[i-1]='=';
        break;
    }
    return res;
}

jbyteArray dumpMethod_code_item(JNIEnv *env, ArtMethod *artMethod) {
    CodeItem * code_item = reinterpret_cast<CodeItem *>(ArtMethod::GetCodeItem(artMethod));
    if (LIKELY(code_item != nullptr)) {
        uint32_t code_item_len = 0;
        uint8_t * item = (uint8_t *)code_item;
        if (code_item->tries_size_ > 0) {
            const uint8_t * handler_data = get_encoded_catch_handler_list(code_item);
            uint8_t *tail = codeitem_end(&handler_data);
            code_item_len = (int)(tail - item);
        } else {
            code_item_len = 16 + code_item->insns_size_in_code_units_ * 2;
        }

        //uint8_t *code_item = reinterpret_cast<uint8_t *>(GetCodeItem_fun(artMethod));

        std::string name = runtime::ArtMethod_PrettyMethod((void *)artMethod, false);
        uint32_t method_idx = artMethod->GetDexMethodIndex();
        auto dexFile = ArtMethod::GetDexFile(artMethod);
        auto begin_ = dexFile->begin_;
        auto offset = item - begin_;
        //dump(dexFile->begin_, name.c_str(), method_idx, offset, item, code_item_len, dump_dir);
        long outlen = 0;
        char * ins_code = base64_encode((char *)item, code_item_len, &outlen);
        char * result = (char *)malloc(outlen + 1000);
        memset(result, 0, outlen + 1000);
        sprintf(result, "{name:%s,method_idex:%d,offset:%d,code_item_len:%d,ins:%s}\n", name.c_str(), method_idx, offset, code_item_len, ins_code);
        auto size = strlen(result) + 1;
        jbyteArray  jbyteArray1 = env->NewByteArray(size);
        env->SetByteArrayRegion(jbyteArray1, 0, size, reinterpret_cast<const jbyte *>(result));
        free(ins_code);
        free(result);
        return jbyteArray1;
        //jbyteArray byteArray = env->NewByteArray(code_item_len);
        //env->SetByteArrayRegion(byteArray, 0, code_item_len, reinterpret_cast<const jbyte *>(code_item));
        //return byteArray;
    }
    return nullptr;
}

jbyteArray dumpMethodByMember(JNIEnv *env, jclass cls, jobject method) {
    ArtMethod *artMethod = GetArtMethod(env, cls, method);
    return dumpMethod_code_item(env, artMethod);
}

class Dumper {
private:
    const char * dump_dir;
    std::map<const uint8_t *, int> fpmap;
    char * encode_buffer;
    int encode_buffer_count;
    char * result_buffer;
    int result_buffer_count;

public:
    Dumper(const char * dump_dir) {
        this->dump_dir = dump_dir;
        this->fpmap = std::map<const uint8_t *, int>();
        this->encode_buffer = nullptr;
        this->encode_buffer_count = 0;
        this->result_buffer = nullptr;
        this->result_buffer_count = 0;
        alloc_encode_buffer(102400);
        alloc_result_buffer(102400+1024);
    }

    const char * get_dump_dir() {
        return this->dump_dir;
    }

    char * alloc_encode_buffer(int count) {
        if (count > this->encode_buffer_count) {
            free_encode_buffer();
            this->encode_buffer = (char *)malloc(count);
            this->encode_buffer_count = count;
        }
        return this->encode_buffer;
    }

    void free_encode_buffer() {
        if (this->encode_buffer != nullptr) {
            free(this->encode_buffer);
            this->encode_buffer = nullptr;
            this->encode_buffer_count = 0;
        }
    }

    char * alloc_result_buffer(int count) {
        if (count > this->result_buffer_count) {
            free_result_buffer();
            this->result_buffer = (char *)malloc(count);
            this->result_buffer_count = count;
        }
        return this->result_buffer;
    }

    void free_result_buffer() {
        if (this->result_buffer != nullptr) {
            free(this->result_buffer);
            this->result_buffer = nullptr;
            this->result_buffer_count = 0;
        }
    }

    char * encode_data(char *str,long str_len, long* outlen){
        long len;
        char *res;
        int i,j;
        const char * base64_table="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        if (str_len % 3 == 0) {
            len = str_len / 3 * 4;
        } else {
            len = (str_len / 3 + 1) * 4;
        }
        res = alloc_encode_buffer(sizeof(char)*(len+1));
        res[len] = '\0';
        *outlen = len;
        for(i = 0, j = 0; i < len-2; j += 3, i += 4) {
            res[i] = base64_table[str[j] >> 2];
            res[i+1] = base64_table[(str[j] & 0x3) << 4 | (str[j+1] >> 4)];
            res[i+2] = base64_table[(str[j+1] & 0xf) << 2 | (str[j+2] >> 6)];
            res[i+3] = base64_table[str[j+2] & 0x3f];
        }
        switch (str_len % 3) {
            case 1:
                res[i-2]='=';
                res[i-1]='=';
                break;
            case 2:
                res[i-1]='=';
                break;
        }
        return res;
    }

    const char * make_data(const char * name, uint32_t method_idx, int offset, uint8_t * item, uint32_t code_item_len, int * count) {
        long outlen = 0;
        char * ins_code = encode_data((char *)item, code_item_len, &outlen);
        char * result = (char *)alloc_result_buffer(outlen + 1000);
        memset(result, 0, outlen + 1000);
        *count = sprintf(result, "{name:%s,method_idx:%d,offset:%d,code_item_len:%d,ins:%s}\n", name, method_idx, offset, code_item_len, ins_code);
        return result;
    }

    int insert_item(const uint8_t * begin_) {
        int fp = -1;
        auto it = this->fpmap.find(begin_);
        if (it == this->fpmap.end()) {
            char pathname[1024];
            //memset(pathname, 0, 1024);
            snprintf(pathname, sizeof(pathname), "%s/%p.idx\x00", this->dump_dir, begin_);
            fp = open(pathname, O_RDWR | (O_APPEND | O_CREAT), S_IRWXU);
            if (fp == -1) {
                return fp;
            }
            this->fpmap.emplace(begin_, fp);
        } else {
            fp = it->second;
        }
        return fp;
    }

    void dump(const uint8_t * begin_, const char * name, uint32_t method_idx, int offset, uint8_t * item, uint32_t code_item_len) {
        int fp = insert_item(begin_);
        if (fp == -1) {
            return ;
        }
        int count = 0;
        const char * data = make_data(name, method_idx, offset, item, code_item_len, &count);
        lseek(fp, 0, SEEK_END);
        write(fp, data, count);
    }

    void close_all() {
        free_encode_buffer();
        free_result_buffer();
        for(const auto& kv : this->fpmap) {
            close(kv.second);
        }
        this->fpmap.clear();
    }
};

void dumpArtMethod(Dumper * dumper, ArtMethod *artMethod) {
    CodeItem * code_item = reinterpret_cast<CodeItem *>(ArtMethod::GetCodeItem(artMethod));
    if (LIKELY(code_item == nullptr)) {
        return;
    }
    uint8_t * item = (uint8_t *)code_item;
    std::string name = runtime::ArtMethod_PrettyMethod((void *)artMethod, false);
    uint32_t method_idx = artMethod->GetDexMethodIndex();
    auto dexFile = ArtMethod::GetDexFile(artMethod);
    auto begin_ = dexFile->begin_;
    auto offset = item - begin_;
    uint32_t code_item_len = 0;
    if (code_item->tries_size_ > 0) {
        const uint8_t * handler_data = get_encoded_catch_handler_list(code_item);
        uint8_t *tail = codeitem_end(&handler_data);
        code_item_len = (int)(tail - item);
    } else {
        code_item_len = 16 + code_item->insns_size_in_code_units_ * 2;
    }
    save_dex_file(dumper->get_dump_dir(), dexFile);
    dumper->dump(dexFile->begin_, name.c_str(), method_idx, offset, item, code_item_len);
}

void dumpMethods(JNIEnv *env, jclass clazz, jobjectArray methods, jobjectArray constructors, jstring jdumpDir) {
    std::string dumpDir = env->GetStringUTFChars(jdumpDir, nullptr);
    int nmethods = env->GetArrayLength(methods);
    Dumper * dumper = new Dumper(dumpDir.c_str());
    for (int i = 0; i < nmethods; i ++) {
        jobject method = env->GetObjectArrayElement(methods, i);
        ArtMethod *artMethod = GetArtMethod(env, clazz, method);
        if (artMethod->IsNative() || artMethod->IsAbstract()) {
            continue;
        }
        dumpArtMethod(dumper, artMethod);
    }
    int nctors = env->GetArrayLength(constructors);
    for (int i = 0; i < nctors; i ++) {
        jobject ctor = env->GetObjectArrayElement(constructors, i);
        ArtMethod *artMethod = GetArtMethod(env, clazz, ctor);
        if (artMethod->IsNative() || artMethod->IsAbstract()) {
            continue;
        }
        dumpArtMethod(dumper, artMethod);
    }
    dumper->close_all();
    free(dumper);
}

jmethodID NativeFindJmethod(JNIEnv *env, jclass cls, jstring jmethod_name, jstring jmethod_sign) {
    const char *method_name = env->GetStringUTFChars(jmethod_name, nullptr);
    const char *method_sign = env->GetStringUTFChars(jmethod_sign, nullptr);
    jmethodID ret_jmethod = env->GetStaticMethodID(cls, method_name, method_sign);
    //找不大方法需要清理异常，否则会崩溃
    if (ret_jmethod == nullptr) {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        ret_jmethod = env->GetMethodID(cls, method_name, method_sign);
        if (ret_jmethod == nullptr) {
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
            }
        }
    }
    return ret_jmethod;
}

jbyteArray dumpMethodByString(JNIEnv *env, jclass thiz, jclass cls, jstring method_name, jstring method_sign) {
    jmethodID findMethod = NativeFindJmethod(env, cls, method_name, method_sign);
    ArtMethod *artMethod = GetArtMethod(env, cls, findMethod);
    return dumpMethod_code_item(env, artMethod);
}

