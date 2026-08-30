; ---------------------------------------------------------------------------
;  Loader stub for the packed build.
;
;  dyld binds exactly two symbols for us, dlopen and dlsym, which is all the
;  intro needs to find everything else.  Decompression is oneKpaq - a
;  context-mixing packer in the Crinkler family - so there is no system
;  decompressor to look up either: the decoder is inlined below.
;
;  The intro is unpacked into the zero-filled tail of the same __TEXT segment,
;  past the end of the file, so no allocation is needed. oneKpaq destroys its
;  input while decoding, which is fine: __TEXT is mapped read/write/execute.
; ---------------------------------------------------------------------------

bits 64
default rel
org STUBVA
[map symbols build/stub.map]

%ifndef USE_ONEKPAQ
  %define USE_ONEKPAQ 0
%endif

_stub:
    and     rsp, -16
    mov     r14, [p_dlsym]              ; both survive the decoder, which
    mov     r15, [p_dlopen]             ; only touches rax..rbp and xmm0/1

%if USE_ONEKPAQ
    finit                               ; oneKpaq wants a clean x87 stack
    lea     rbx, [_stub + (PACKEDVA + PACKEDOFF - STUBVA)]
    lea     rdi, [_stub + (DESTVA - STUBVA)]
%include "onekpaq_decompressor64.asm"
%else
    ; Fallback when the oneKpaq encoder is not available: the system LZMA,
    ; which costs about 300 bytes more in the finished binary.
    lea     rdi, [s_libcompression]
    mov     esi, 1
    call    r15
    mov     rdi, -2
    lea     rsi, [s_decode]
    call    r14
    lea     rdi, [_stub + (DESTVA - STUBVA)]
    mov     esi, RAWSZ
    lea     rdx, [_stub + (PACKEDVA - STUBVA)]
    mov     ecx, PACKEDSZ
    xor     r8d, r8d
    mov     r9d, 0x306                  ; COMPRESSION_LZMA
    call    rax
%endif

    mov     rdi, r14                    ; hand the two resolvers over
    mov     rsi, r15
    lea     rax, [_stub + (DESTVA + ENTRY - STUBVA)]
    jmp     rax

%if USE_ONEKPAQ == 0
s_libcompression:   db "libcompression.dylib", 0
s_decode:           db "compression_decode_buffer", 0
%endif
align 8
p_dlsym:            dq 0                ; filled by dyld
p_dlopen:           dq 0
_stub_end:
