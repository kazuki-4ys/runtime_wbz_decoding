#include "assets.h"
#include "patch.h"

extern "C" {
    void dvd_archive_decompress_hook_asm(void);
    void dvd_archive_decompress_hook_asm2(void);
    void dvd_archive_decompress_hook_asm2_end(void);
    void System__DvdArchive__decompress(void *self, const char* path, void* archiveHeap, unsigned int _unused);
    void System__CourseCache__load(void *self, unsigned int courseId);
    unsigned int EGG__Decomp__DecodeSZS(unsigned char *src, unsigned char *dest);
    unsigned int EGG__Decomp__DecodeSZS_Original(unsigned char *src, unsigned char *dest);
    unsigned int EGG__Decomp__getExpandSize(unsigned char *src);
    extern const char *auto_add_not_found_msg_jpn_bin;
    #include "wu8_decode.h"
    #include "libbz2/bzlib.h"
    #include "liblzma/LzmaDec.h"
}

#include <types.h>

namespace mod {

void *decodeSzsHeap;
void *lzmaHeap;

void System__CourseCache__load_Replace(void *self, unsigned int courseId){
    //disable course cache
    return;
}

void* my_lzma_malloc(void *p, unsigned int size){
    return my_malloc_from_heap(size, lzmaHeap);
}

void my_lzma_free(void *p, void *ptr){
    Egg__Heap__Free(ptr, lzmaHeap);
}

unsigned char isFileValid(const char *path){
    DVDFileInfo fi;
    int result = DVDOpen(path, &fi);
    if(!result)return 0;
    unsigned int length = fi.length;
    DVDClose(&fi);
    if(!length)return 0;
    return 1;
}

void decompressLzma(unsigned char *src, unsigned int srcSize, unsigned char *dest, unsigned int destSize, void *heap){
    lzmaHeap = heap;
    ELzmaStatus status;
    ISzAlloc isza;
    isza.Alloc = ((void* (*)(ISzAllocPtr, size_t))(void*)my_lzma_malloc);
    isza.Free = (void (*)(ISzAllocPtr, void*))((void*)my_lzma_free);
    srcSize = srcSize - 13;
    int ret = (int)LzmaDecode((Byte*)dest, (SizeT*)(&destSize), (Byte*)(src + 13), (SizeT*)(&srcSize), (Byte*)src, 5, LZMA_FINISH_END, &status, &isza);
}

extern "C" {
void bz_internal_error(int errcode){
}
}

void decompressBz2(unsigned char *src, unsigned int srcSize, unsigned char *dest, unsigned int destSize, void *heap){
    bzHeap = heap;
    bz_internal_error(BZ2_bzBuffToBuffDecompress((char*)dest, &destSize, (char*)src, srcSize, 0, 4));
}

extern "C" {
unsigned int DvdArchiveDecompressHook(unsigned char *fileStart, void *heap, unsigned int sourceSize){
    //replace here.
    //https://github.com/riidefi/mkw/blob/master/source/game/system/DvdArchive.cpp#L222
    decodeSzsHeap = heap;
    if(!memcmp(fileStart, "Yaz", 3))return EGG__Decomp__getExpandSize(fileStart);
    //Assume that this is wbz or wlz.
    unsigned int result;
    memcpy(&result, fileStart + 0xC, 4);
    *((unsigned int*)((void*)(fileStart + 0x8))) = sourceSize;
    return result;
}

unsigned int dvd_archive_decompress_hook2(unsigned char *src, unsigned char *dest){
    //replace here.
    //https://github.com/riidefi/mkw/blob/40c587abb0bb386532aaf038e290524c86ab4c1f/source/egg/core/eggDecomp.cpp#L35
    if(!memcmp(src, "Yaz", 3)){
        if(!memcmp(src + 0x10, "\x42\x5a\x68", 3)){
            decompressBz2(src + 0x10, *((unsigned int*)((void*)(src + 0x8))), dest, *((unsigned int*)((void*)(src + 0x4))), decodeSzsHeap);
            return *((unsigned int*)((void*)(src + 0x4)));
        }
        if(!memcmp(src + 0x10, "\x5d\x00\x00", 3)){
            decompressLzma(src + 0x10, *((unsigned int*)((void*)(src + 0x8))), dest, *((unsigned int*)((void*)(src + 0x4))), decodeSzsHeap);
            return *((unsigned int*)((void*)(src + 0x4)));
        }
        return 0;
    }
    if(!memcmp(src, "WBZa", 4))decompressBz2(src + 0x10, *((unsigned int*)((void*)(src + 0x8))) - 0x10, dest, *((unsigned int*)((void*)(src + 0xC))), decodeSzsHeap);
    if(!memcmp(src, "WLZa", 4))decompressLzma(src + 0x10, *((unsigned int*)((void*)(src + 0x8))) - 0x10, dest, *((unsigned int*)((void*)(src + 0xC))), decodeSzsHeap);
    if(!memcmp(dest, "WU8a", 4))decode_wu8(dest, *((unsigned int*)((void*)(src + 0xC))), decodeSzsHeap);
    return *((unsigned int*)((void*)(src + 0xC)));
}

}

unsigned int EGG__Decomp__getExpandSize_Replace(unsigned char *src){
    if(!memcmp(src, "Yaz", 3)){
        return *((unsigned int*)((void*)(src + 4)));
    }
    //Assume that this is wbz or wlz.
    return *((unsigned int*)((void*)(src + 0xC)));
}

void main()
{
    // Replace required function
    patch::hookFunction(System__CourseCache__load, System__CourseCache__load_Replace);
    patch::hookFunction((void (*)())((void*)(((unsigned char*)((void*)System__DvdArchive__decompress)) + 0x28)), dvd_archive_decompress_hook_asm);
    patch::hookFunction((void (*)())((void*)EGG__Decomp__DecodeSZS), dvd_archive_decompress_hook_asm2);
    patch::hookFunction(dvd_archive_decompress_hook_asm2_end, (void (*)())((void*)(((unsigned char*)((void*)EGG__Decomp__DecodeSZS)) + 4)));
    patch::hookFunction(EGG__Decomp__getExpandSize, EGG__Decomp__getExpandSize_Replace);

}

}
