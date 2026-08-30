bits 64
default rel
section .text
global _okp_decompress
; void okp_decompress(void *src /*rdi*/, void *dst /*rsi*/)
_okp_decompress:
    push    rbx
    push    rbp
    push    r12
    push    r13
    mov     rbx, rdi                    ; src (already at +offset)
    mov     rdi, rsi                    ; dst
    finit                               ; the decoder wants a clean x87 stack
%include "onekpaq_decompressor64.asm"
    pop     r13
    pop     r12
    pop     rbp
    pop     rbx
    ret
