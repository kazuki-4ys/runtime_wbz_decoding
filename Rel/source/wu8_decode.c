#include "wu8_decode.h"

#define U8_MAGIC 0x55AA382D

void *Egg__Heap__Alloc(unsigned int, unsigned int, void*);

void *my_malloc_from_heap(unsigned int length, void *heap){
    unsigned int requsetLength = length;
    if(requsetLength & 0x1F){//0x20でアラインメント alignment for 0x20
        requsetLength = ((requsetLength >> 5) + 1) << 5;
    }
    void *dest = Egg__Heap__Alloc(requsetLength, 0x20, heap);
    //OSReport("[KZ-RTD]: memory allocated at: 0x%08x\n", dest);
    return dest;
}

char *addDotSlashIfNotExist(char *srcPath, void *heap){
    if((strlen(srcPath) < 2) || memcmp("./", srcPath, 2)){
        char *dest = my_malloc_from_heap(strlen(srcPath) + 3, heap);
        memcpy(dest + 2, srcPath, strlen(srcPath) + 1);
        memcpy(dest, "./", 2);
        return dest;
    }
    return NULL;
}

void u8_archive_init_auto_add(u8_archive *src, const char *path, void *heap){
    src->heap = heap;
    src->nodeAndStringTable = NULL;
    if(!DVDOpen(path, &(src->fi)))return;
    void *tmpHeader = my_malloc_from_heap(0x10, heap);
    DVDReadPrio(&(src->fi), tmpHeader, 0x10, 0, 2);
    memcpy(&(src->header), tmpHeader, 0x10);
    Egg__Heap__Free(tmpHeader, heap);
    src->nodeAndStringTable = my_malloc_from_heap(src->header.nodeAndStringTableSize, heap);
    DVDReadPrio(&(src->fi), src->nodeAndStringTable, src->header.nodeAndStringTableSize, src->header.firstNodeOffset, 2);

    //for test
    /*char *tmpFullPath = my_malloc_from_heap(1024, heap);
    u8_node *firstNode = (void*)(src->nodeAndStringTable);
    u8_node *targetNode = NULL;
    char *fullPath = NULL;
    unsigned int allNodeCount = firstNode->fileDataLength;
    for(unsigned int i = 0;i < allNodeCount;i++){
        if(i == 0)continue;
        targetNode = firstNode + i;
        fullPath = u8_get_full_path(src, i, tmpFullPath);
        OSReport("%s\n", fullPath);
    }
    Egg__Heap__Free(tmpFullPath, src->heap);*/
}

unsigned char u8_archive_is_this_node_empty(u8_archive *src, unsigned int targetNodeIndex){
    u8_node *firstNode = (void*)(src->nodeAndStringTable);
    u8_node *targetNode = firstNode + targetNodeIndex;
    unsigned int allNodeCount = firstNode->fileDataLength;
    if((targetNode->fileNameOffset & 0x01000000) != 0x01000000){
        //this is a file.
        return 0;
    }
    if(targetNode->fileDataLength <= targetNodeIndex + 1)return 1;
    return 0;
}

void u8_archive_deinit_auto_add(u8_archive *src){
    if(!src->nodeAndStringTable)return;
    Egg__Heap__Free(src->nodeAndStringTable, src->heap);
    DVDClose(&(src->fi));
}

void getFullPath_rec(u8_archive *src, unsigned int targetNodeIndex, u8_node_str_array *pathInfo){
    unsigned int parentNodeIndex = 0;
    u8_node *firstNode = (void*)(src->nodeAndStringTable);
    u8_node *targetNode = firstNode + targetNodeIndex;
    u8_node *parentNode = NULL;
    unsigned int allNodeCount = firstNode->fileDataLength;
    if(pathInfo->count >= U8_NODE_STR_MAX_COUNT)return;
    if((targetNode->fileNameOffset & 0x01000000) == 0x01000000){
        parentNodeIndex = targetNode->fileDataOffset;
        parentNode = firstNode + parentNodeIndex;
        //ここから合っているかは不明
        while(parentNodeIndex < allNodeCount){
            parentNode = firstNode + parentNodeIndex;
            //if((parentNode->fileNameOffset & 0x01000000) == 0x01000000 && (!u8_archive_is_this_node_empty(src, parentNodeIndex)))break;
            if((parentNode->fileNameOffset & 0x01000000) == 0x01000000){
                if(targetNodeIndex < parentNode->fileDataLength)break;
                parentNodeIndex = parentNode->fileDataLength - 1;
            }
            parentNodeIndex++;
        }
        if(parentNodeIndex >= allNodeCount){
            parentNodeIndex = allNodeCount - 1;
            parentNode = firstNode + parentNodeIndex;
        }
    }else{
        parentNodeIndex = targetNodeIndex - 1;
        while(1){
            parentNode = firstNode + parentNodeIndex;
            if((parentNode->fileNameOffset & 0x01000000) == 0x01000000){
                if(parentNode->fileDataLength > targetNodeIndex)break;
            }
            if(!parentNodeIndex)break;
            parentNodeIndex--;
        }
    }
    pathInfo->str[pathInfo->count] = src->nodeAndStringTable + (targetNode->fileNameOffset & 0xFFFFFF) + allNodeCount * U8_NODE_SIZE;
    pathInfo->count++;
    if(parentNodeIndex > 0)getFullPath_rec(src, parentNodeIndex, pathInfo);
    return;
}

char* u8_get_full_path(u8_archive *src, unsigned int targetNodeIndex){
    u8_node_str_array *pathInfo;
    pathInfo = my_malloc_from_heap(sizeof(u8_node_str_array), src->heap);
    pathInfo->count = 0;
    getFullPath_rec(src, targetNodeIndex, pathInfo);
    if(pathInfo->count == 0){
        Egg__Heap__Free(pathInfo, src->heap);
        return NULL;
    }
    unsigned int slashCount = pathInfo->count - 1;
    unsigned int pathBufLength = slashCount + 1;
    for(unsigned int i = 0;i < pathInfo->count;i++){
        pathBufLength += strlen(pathInfo->str[i]);
    }
    char *dest = my_malloc_from_heap(pathBufLength, src->heap);
    unsigned int curDestIndex = 0;
    for(unsigned int i = 0;i < pathInfo->count;i++){
        unsigned int curStrLen = strlen(pathInfo->str[(pathInfo->count - 1) - i]);
        memcpy(dest + curDestIndex, pathInfo->str[(pathInfo->count - 1) - i], curStrLen);
        curDestIndex += curStrLen;
        if(i + 1 == pathInfo->count){
            *(dest + curDestIndex) = 0;
        }else{
            *(dest + curDestIndex) = '/';
            curDestIndex++;
        }
    }
    Egg__Heap__Free(pathInfo, src->heap);
    return dest;
}

unsigned char u8_archive_is_file_exist_auto_add(u8_archive *src, const char *path){
    unsigned char result = 0;
    u8_node *firstNode = (void*)(src->nodeAndStringTable);
    u8_node *targetNode = NULL;
    char *fullPath = NULL;
    unsigned int allNodeCount = firstNode->fileDataLength;
    for(unsigned int i = 0;i < allNodeCount;i++){
        fullPath = u8_get_full_path(src, i);
        if(fullPath && (!strcmp(fullPath, path))){
            result = 1;
        }
        if(fullPath)Egg__Heap__Free(fullPath, src->heap);
        if(result)return 1;
    }
    return 0;
}

void u8_archive_get_file_auto_add(u8_archive *src, const char *path, u8_archive_auto_add_get_file_dest *dest){
    dest->data = NULL;
    dest->size = 0;
    if(!src->nodeAndStringTable)return;
    u8_node *firstNode = (void*)(src->nodeAndStringTable);
    u8_node *targetNode = NULL;
    char *fullPath = NULL;
    unsigned int allNodeCount = firstNode->fileDataLength;
    for(unsigned int i = 0;i < allNodeCount;i++){
        if(i == 0)continue;
        targetNode = firstNode + i;
        if((targetNode->fileNameOffset & 0x01000000) == 0x01000000)continue;
        fullPath = u8_get_full_path(src, i);
        if((!fullPath) || strcmp(fullPath, path)){
            if(fullPath)Egg__Heap__Free(fullPath, src->heap);
            continue;
        }
        Egg__Heap__Free(fullPath, src->heap);
        dest->data = my_malloc_from_heap(targetNode->fileDataLength, src->heap);
        dest->heap = src->heap;
        dest->size = targetNode->fileDataLength;
        unsigned int offset = targetNode->fileDataOffset;
        DVDReadPrio(&(src->fi), dest->data, dest->size, offset, 2);
        return;
    }
}

unsigned char getBasickeyBySize(unsigned int srcSize){
    unsigned char tmp[4];
    memcpy(tmp, &srcSize, 4);
    return tmp[0] ^ tmp[1] ^ tmp[2] ^ tmp[3];
}

void decode_wu8(unsigned char *src, unsigned int srcSize, void *heap){
    //https://wiki.tockdom.com/wiki/WU8_(File_Format)#Algorithm
    u8_archive src_wu8, auto_add;
    src_wu8.heap = heap;
    u8_archive_init_auto_add(&auto_add, "/Race/Course/auto-add.arc", heap);
    //Algorithm 6
    unsigned int tmp = U8_MAGIC;
    memcpy(src, &tmp, 4);
    memcpy(&(src_wu8.header), src, 16);
    src_wu8.nodeAndStringTable = src + src_wu8.header.firstNodeOffset;
    u8_node *firstNode = (void*)src_wu8.nodeAndStringTable;
    //Algorithm 1
    unsigned char basicKey = getBasickeyBySize(srcSize);
    unsigned char summaryKey = basicKey;
    //Algorithm 5
    for(unsigned int i = 0;i < src_wu8.header.nodeAndStringTableSize;i++)src_wu8.nodeAndStringTable[i] = src_wu8.nodeAndStringTable[i] ^ basicKey;
    //Algorithm 2 and 3
    unsigned int allNodeCount = firstNode->fileDataLength;
    for(unsigned int i = 1;i < allNodeCount;i++){
        char *fullPath;
        char *dotSlashFullPath;
        unsigned char freeDotSlashFullPath = 1;
        u8_archive_auto_add_get_file_dest origFile;
        if(((firstNode + i)->fileNameOffset & 0x01000000) == 0x01000000)continue;
        unsigned int fileSize = (firstNode + i)->fileDataLength;
        unsigned char *file = src + (firstNode + i)->fileDataOffset;
        fullPath = u8_get_full_path(&src_wu8, i);
        dotSlashFullPath = addDotSlashIfNotExist(fullPath, heap);
        if(!dotSlashFullPath){
            dotSlashFullPath = fullPath;
            freeDotSlashFullPath = 0;
        }
        //OSReport("%s\n", fullPath);
        if(!u8_archive_is_file_exist_auto_add(&auto_add, dotSlashFullPath)){
            if(freeDotSlashFullPath)Egg__Heap__Free(dotSlashFullPath, heap);
            Egg__Heap__Free(fullPath, src_wu8.heap);
            continue;
        }
        u8_archive_get_file_auto_add(&auto_add, dotSlashFullPath, &origFile);
        // Algorithm 3
        //https://github.com/Wiimm/wiimms-szs-tools/blob/336da838ce43edae40dd708dd465f1f43a2c312d/project/src/lib-szs.c#L1795
        summaryKey ^= origFile.data[origFile.size / 2] ^ origFile.data[origFile.size / 3] ^ origFile.data[origFile.size / 4];
        for(unsigned int j = 0;j < fileSize;j++){
            *(file + j) = basicKey ^ (*(file + j)) ^ (*(origFile.data + (j % origFile.size)));
        }
        Egg__Heap__Free(origFile.data, origFile.heap);
        if(freeDotSlashFullPath)Egg__Heap__Free(dotSlashFullPath, heap);
        Egg__Heap__Free(fullPath, src_wu8.heap);
    }
    // Algorithm 4
    for(unsigned int i = 1;i < allNodeCount;i++){
        char *fullPath;
        char *dotSlashFullPath;
        unsigned char freeDotSlashFullPath = 1;
        if(((firstNode + i)->fileNameOffset & 0x01000000) == 0x01000000)continue;
        unsigned int fileSize = (firstNode + i)->fileDataLength;
        unsigned char *file = src + (firstNode + i)->fileDataOffset;
        fullPath = u8_get_full_path(&src_wu8, i);
        dotSlashFullPath = addDotSlashIfNotExist(fullPath, heap);
        if(!dotSlashFullPath){
            dotSlashFullPath = fullPath;
            freeDotSlashFullPath = 0;
        }
        unsigned char u8_archive_is_file_exist_auto_add_result = u8_archive_is_file_exist_auto_add(&auto_add, dotSlashFullPath);
        if(freeDotSlashFullPath)Egg__Heap__Free(dotSlashFullPath, heap);
        Egg__Heap__Free(fullPath, src_wu8.heap);
        if(u8_archive_is_file_exist_auto_add_result)continue;
        for(unsigned int j = 0;j < fileSize;j++)*(file + j) = summaryKey ^ (*(file + j));
    }
    u8_archive_deinit_auto_add(&auto_add);
}