#ifdef RMCJ

#define IC_INVALIDATE_RANGE 0x801a1630
#define OSREPORT 0x801A24F0
#define EGG_HEAP_ALLOC 0x80229734
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015DE6C
#define DVD_FAST_OPEN 0x8015E174
#define DVD_READ_PRIO 0x8015E754
#define DVD_CLOSE 0x8015E488
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000F238
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6C50
#define OSLINK 0x801a71fc
#define FST_STRING_START 0x80385FBC
#define FST_START 0x80385FC0
#define MAX_ENTRY_NUM 0x80385FB8
#define STRLEN 0x80021178
#define STRCMP 0x800131c0
#define SPRINTF 0x80011950

#endif
#ifdef RMCE

#define IC_INVALIDATE_RANGE 0x801a1670
#define OSREPORT 0x801a2530
#define EGG_HEAP_ALLOC 0x80229490
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015deac
#define DVD_FAST_OPEN 0x8015e1b4
#define DVD_READ_PRIO 0x8015e794
#define DVD_CLOSE 0x8015e4c8
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000e7b4
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6C90
#define OSLINK 0x801a723c
#define FST_STRING_START 0x803822BC
#define FST_START 0x803822C0
#define MAX_ENTRY_NUM 0x803822B8
#define STRLEN 0x800206f4
#define STRCMP 0x8001273c
#define SPRINTF 0x80010ecc

#endif
#ifdef RMCP

#define IC_INVALIDATE_RANGE 0x801a1710
#define OSREPORT 0x801a25d0
#define EGG_HEAP_ALLOC 0x80229814
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015df4c
#define DVD_FAST_OPEN 0x8015e254
#define DVD_READ_PRIO 0x8015e834
#define DVD_CLOSE 0x8015e568
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000f314
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6D30
#define OSLINK 0x801a72dc
#define FST_STRING_START 0x8038663c
#define FST_START 0x80386640
#define MAX_ENTRY_NUM 0x80386638
#define STRLEN 0x80021254
#define STRCMP 0x8001329c
#define SPRINTF 0x80011a2c

#endif

#ifdef RMCK

#define IC_INVALIDATE_RANGE 0x801a1a6c
#define OSREPORT 0x801a292c
#define EGG_HEAP_ALLOC 0x80229b88
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015dfc4
#define DVD_FAST_OPEN 0x8015e2cc
#define DVD_READ_PRIO 0x8015e8ac
#define DVD_CLOSE 0x8015e5e0
#define MEMCPY 0x80005f34
#define MEMCMP 0x8000f37c
#define LE_CODE_LOADER_INJECT_ADDR 0x801a708c
#define OSLINK 0x801a7638
#define STRLEN 0x800212bc
#define STRCMP 0x80013304
#define SPRINTF 0x80011a94

#endif

typedef struct{
    unsigned char unk0[0x34];
    unsigned int length;
    //0x38
    unsigned char unk1[4];
    //全部で0x3Cバイト
} DVDFileInfo;

void *getSystemHeap(void);
void callRelProlog(void *rel);
unsigned int getRelBssSize(void *rel);
void *get_data0(void);

void *my_malloc(unsigned int length){
    void* (*Egg__Heap__Alloc)(unsigned int, unsigned int, void*) = (void*)EGG_HEAP_ALLOC;
    unsigned int requsetLength = length;
    if(requsetLength & 0x1F){//0x20でアラインメント alignment for 0x20
        requsetLength = ((requsetLength >> 5) + 1) << 5;
    }
    return Egg__Heap__Alloc(requsetLength, 0x20, getSystemHeap());
}

void __main(void){
    int (*DVDConvertPathToEntryNum)(const char*) = (void*)DVD_CONVERT_PATH_TO_ENTRY_NUM;
    int (*DVDFastOpen)(int, DVDFileInfo*) = (void*)DVD_FAST_OPEN;
    int (*DVDReadPrio)(DVDFileInfo*, void*, unsigned int, unsigned int, unsigned int) = (void*)DVD_READ_PRIO;
    void (*DVDClose)(DVDFileInfo*) = (void*)DVD_CLOSE;
    void (*OSLink)(void*, void*) = (void*)OSLINK;
    DVDFileInfo relFileInfo;
    if(DVDFastOpen(DVDConvertPathToEntryNum(get_data0()), &relFileInfo) == 0)return;
    void *relFile = my_malloc(relFileInfo.length);
    DVDReadPrio(&relFileInfo, relFile, relFileInfo.length, 0, 2);
    DVDClose(&relFileInfo);
    void *relBss = my_malloc(getRelBssSize(relFile));
    OSLink(relFile, relBss);
    callRelProlog(relFile);
}