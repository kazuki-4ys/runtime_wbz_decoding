#ifndef _COMMON_H_
#define _COMMON_H_

typedef struct{
    unsigned char unk0[0x34];
    unsigned int length;
    //0x38
    unsigned char unk1[4];
    //全部で0x3Cバイト
} DVDFileInfo;

#define NULL 0

void OSFatal(unsigned int *strCol, unsigned int *bgCol, const char *str);
void OSReport(const char*, ...);
void memcpy(void*, void*, unsigned int);
void *memset(void *buf, int ch, unsigned int n);
int snprintf(char* s, unsigned int n, const char* format, ...);
int strlen(const char*);
int strcmp(const char*, const char*);
int memcmp(const void *buf1, const void *buf2, unsigned int count);
void *my_malloc_from_heap(unsigned int length, void *heap);
int DVDConvertPathToEntrynum(const char*);
int DVDOpen(const char *fileName, DVDFileInfo *fileInfo);
int DVDReadPrio(DVDFileInfo *fileInfo, void *addr, unsigned int length, unsigned int offset, unsigned int prio);
int DVDClose(DVDFileInfo *fileInfo);
void Egg__Heap__Free(void* ptr, void *heap);

#endif//_COMMON_H_