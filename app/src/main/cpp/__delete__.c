
void * libart_module = NULL;

extern "C" typedef uint64_t (*fun_art_ArtMethod_GetObsoleteDexCache)(void *);
void * orig_art_ArtMethod_GetObsoleteDexCache = NULL;

extern "C" typedef std::string (*fun_art_ArtMethod_PrettyMethod)(void *, bool);
void * orig_art_ArtMethod_PrettyMethod = NULL;

extern "C" typedef void * (*fun_NterpGetCodeItem)(void *);
void * orig_NterpGetCodeItem = NULL;

//extern "C" typedef void (*fun_art_ArtMethod_Invoke)(void *, void *, uint32_t *, uint32_t, void *, const char *);
//void * orig_art_ArtMethod_Invoke = NULL;

DexFile * get_dexfile(void * thiz);
void dump_dex_file(DexFile * dex_file);

void * get_code_item(void * art_method) {
    uint32_t * p1 = (uint32_t *)art_method;
    uint32_t v1 = p1[1];
    uint32_t v2 = p1[2];
    //bool flag1 = (v1 & 0x100) != 0;
    //bool flag2 = (v1 & 0x400) != 0;
    //bool flag3 = (v1 & 0x81400400) == 0x1400400;
    //bool flag4 = v2 == -1;
    //LOGV("flag1: %b, flag2: %b, flag3: %b, flag4: %b", flag1, flag2, flag3, flag4);
    p1[1] = 0;
    p1[2] = 0;
    void * code_item = ((fun_NterpGetCodeItem)orig_NterpGetCodeItem)(art_method);
    p1[1] = v1;
    p1[2] = v2;
    return code_item;
}

/*
extern "C"
void new_art_ArtMethod_Invoke(void *thiz, void *thread, uint32_t *args, uint32_t args_size, void *result, const char *shorty) {
    //LOGV("art_ArtMethod_Invoke(6) enter");
    ((fun_art_ArtMethod_Invoke)orig_art_ArtMethod_Invoke)(thiz, thread, args, args_size, result, shorty);
    DexFile * dex_file = get_dexfile(thiz);
    dump_dex_file(dex_file);
    if (dex_file->IsStandardDexFile()) {
        std::string name = ((fun_art_ArtMethod_PrettyMethod)orig_art_ArtMethod_PrettyMethod)(thiz, false);
        ArtMethod * art_method = reinterpret_cast<ArtMethod *>(thiz);
        if (art_method->IsAbstract() || art_method->IsNative()) {
            return ;
        }
        void * code_item = get_code_item(thiz);
        StandardCodeItem * s_code_item = reinterpret_cast<StandardCodeItem *>(code_item);
        if ((uint8_t *)s_code_item < dex_file->begin_ || (uint8_t *)s_code_item >= dex_file->begin_ + dex_file->size_) {
            return ;
        }
        uint32_t offset = (uint8_t *)s_code_item - (uint8_t *)dex_file->begin_;
        uint32_t method_idx = art_method->GetDexMethodIndex();
        uint32_t code_item_size = s_code_item->GetCodeItemSize();
        LOGV("offset: %x, method_idx: %d, code_item_size: %d", offset, method_idx, code_item_size);
        LOGV("DexFile: %p, %p, CodeItem: %p, %s", dex_file->begin_, dex_file->begin_ + dex_file->size_, s_code_item, name.c_str());
    }
    //LOGV("art_ArtMethod_Invoke(6): %s", name.c_str());
    //LOGV("art_ArtMethod_Invoke(6) leave");
}
/**/


DexFile * get_dexfile(void * thiz) {
    uint16_t word_10 = 3;
    uint64_t v2;
    if ((*((uint32_t *)thiz + 1) & 0x40000) != 0) {
        v2 = ((fun_art_ArtMethod_GetObsoleteDexCache)orig_art_ArtMethod_GetObsoleteDexCache)(thiz);
    } else {
        v2 = *(uint32_t *)(*(uint32_t *)thiz + 16);
    }
    return (DexFile *)(*(uint64_t *)(v2 + 16));
}

extern "C" typedef int (*fun_art_ClassLinker_RegisterDexFile)(void *, DexFile &, int64_t);
void * orig_art_ClassLinker_RegisterDexFile = NULL;

std::map<const uint8_t *, DexFile *> g_dex_files;

bool get_process_name(char * procname, int max_count) {
    char cmdline[1024] = {0};
    int proc_id = getpid();
    sprintf(cmdline, "/proc/%d/cmdline", proc_id);
    int fp = open(cmdline, O_RDONLY, 0644);
    if (fp < 0) {
        return false;
    }
    if (read(fp, procname, max_count) < 0) {
        close(fp);
        return false;
    }
    close(fp);
    return true;
}

bool write_file(char * filename, const uint8_t * data, int size) {
    int fp = open(filename, O_CREAT | O_APPEND | O_RDWR, 0666);
    if (fp < 0) {
        return false;
    }
    if (write(fp, data, size) < 0) {
        close(fp);
        return false;
    }
    fsync(fp);
    close(fp);
    return true;
}

void write_dex_file(const uint8_t * begin, size_t size) {
    char proc_name[1024] = {0};
    if (!get_process_name(proc_name, 1024)) {
        LOGV("get_process_name failed!");
        return ;
    }
    char filename[1024] = {0};
    sprintf(filename, "/data/data/%s/dump/%p.dex", proc_name, begin);
    if (!write_file(filename, begin, size)) {
        LOGV("write_file %s failed!", filename);
        return ;
    }
}

void dump_dex_file(DexFile * dex_file) {
    if (g_dex_files.find(dex_file->begin_) != g_dex_files.end()) {
        return ;
    }
    LOGV("DexFile: begin: %p, size: %lx, standard: %b", dex_file->begin_, dex_file->size_, dex_file->IsStandardDexFile());
    try {
        write_dex_file(dex_file->begin_, dex_file->size_);
    } catch (...) {
        LOGV("occue except! write_dex_file failed!");
    }
    g_dex_files.emplace(dex_file->begin_, dex_file);
}

extern "C"
int new_art_ClassLinker_RegisterDexFile(void * a1, DexFile &dex_file, int64_t a3) {
    //LOGV("art_ClassLinker_RegisterDexFile(3) enter");
    int ret = ((fun_art_ClassLinker_RegisterDexFile)orig_art_ClassLinker_RegisterDexFile)(a1, dex_file, a3);
    //LOGV("art_ClassLinker_RegisterDexFile(3) leave");
    dump_dex_file(&dex_file);
    return ret;
}

extern "C" typedef bool (*fun_art_ConvertJavaArrayToDexFiles)(void *, void *, std::vector<DexFile *>&, void *);
void * orig_art_ConvertJavaArrayToDexFiles = NULL;

void lookup_dex_files(std::vector<DexFile *>& dex_files) {
    //std::vector<DexFile *>& dex_files = reinterpret_cast<std::vector<DexFile *>&>(a3);
    for (auto dex_file : dex_files) {
        dump_dex_file(dex_file);
        //LOGV("base: %x, size: %x", dex_file->begin_, dex_file->size_);
    }
}
extern "C"
bool new_art_ConvertJavaArrayToDexFiles(void * a1, void * a2, std::vector<DexFile *>& a3, void * a4) {
    LOGV("art_ConvertJavaArrayToDexFiles(4) enter");
    bool ret = ((fun_art_ConvertJavaArrayToDexFiles)orig_art_ConvertJavaArrayToDexFiles)(a1, a2, a3, a4);
    lookup_dex_files(a3);
    LOGV("art_ConvertJavaArrayToDexFiles(4) leave");
    return ret;
}