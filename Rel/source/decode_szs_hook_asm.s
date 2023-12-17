#include <ppc-asm.h>

.global dvd_archive_decompress_hook_asm
.global EGG__Decomp__DecodeSZS_Original

.macro pushStack
    stwu sp, -0x80 (sp)#124 + パディング
    mflr r0
    stw r0, 0x84 (sp)
    stmw r3, 8 (sp)
.endm

.macro popStack
    lmw r3, 8 (sp)
    lwz r0, 0x84 (sp)
    mtlr r0
    addi sp, sp, 0x80
.endm

dvd_archive_decompress_hook_asm:
pushStack
lwz r4, 0x1C (r28)
lwz r5, 0x18 (r28)
#r4 = heap
#r5 = mFileSize
bl DvdArchiveDecompressHook
stw r3, -4 (sp)
popStack
lwz r3, -0x84 (sp)
lis r12, System__DvdArchive__decompress@h
ori r12, r12, System__DvdArchive__decompress@l 
addi r12, r12, 0x2C
mtlr r12
blr

EGG__Decomp__DecodeSZS_Original:
stwu sp, -0x10 (sp)
lis r12, EGG__Decomp__DecodeSZS@h
ori r12, r12, EGG__Decomp__DecodeSZS@l 
addi r12, r12, 0x4
mtctr r12
bctr