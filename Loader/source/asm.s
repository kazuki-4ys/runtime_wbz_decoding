#include <ppc-asm.h>
.set region, 'J'

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

.global __entry
.global getSystemHeap
.global blTrickCommonEnd
.global callRelProlog
.global getRelBssSize
.global get_data0

.section .text.__entry
__entry:
    pushStack
    bl __main
    b __end

#by vega
#https://mariokartwii.com/showthread.php?tid=1218
getSystemHeap:
.if    (region == 'E' || region == 'e')
        lwz r3, -0x5CA8(r13)
.elseif (region == 'P' || region == 'p')
        lwz r3, -0x5CA0(r13)
.elseif (region == 'J' || region == 'j')
        lwz r3, -0x5CA0(r13)
.elseif (region == 'K' || region == 'k')
        lwz r3, -0x5C80(r13)
.else
		.abort
.endif
    lwz r3, 0x24(r3)
    blr

callRelProlog:
lwz r12, 0x34 (r3)
mtctr r12
bctr

getRelBssSize:
lwz r3, 0x20 (r3)
blr

blTrickCommonEnd:
    mflr r3
    mtlr r12
    blr

get_data0:
    mflr r12
    bl blTrickCommonEnd
.if    (region == 'E' || region == 'e')
    .string "/rel/runtime_wbz_decoding_E.rel"
.elseif (region == 'P' || region == 'p')
    .string "/rel/runtime_wbz_decoding_P.rel"
.elseif (region == 'J' || region == 'j')
    .string "/rel/runtime_wbz_decoding_J.rel"
.elseif (region == 'K' || region == 'k')
    .string "/rel/runtime_wbz_decoding_K.rel"
.else
		.abort
.endif
    .balign 4

.section .text.__end
#このシンボル__endは必ず最後に配置し、コードハンドラによるbranch命令でゲームのコードに戻れるようにする
__end:
    popStack
    lwz r4, 0x20 (r26)