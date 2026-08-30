; ---------------------------------------------------------------------------
;  oneKpaq decompressor, x86-64 port of onekpaq_decompressor32.asm
;  (original: Copyright (c) Teemu Suutari, BSD-2-Clause)
;
;  Mode 3 only: single section, fast decoder.  The fast variant matches with
;  SSE instead of a byte loop, which removes one of the three nested PUSHAD
;  levels - and PUSHAD is the thing that does not exist in 64-bit mode.
;
;  Differences from the 32-bit original, all forced:
;    PUSHAD/POPAD  -> seven explicit pushes (RSP is not restored by POPAD
;                     either, so nothing is lost)
;    SALC          -> SBB AL,AL, same effect, one byte more
;    pointers      -> 64-bit registers; RBX/RSI/RDI/RDX carry addresses, while
;                     EAX/ECX/EBP/EDX-as-c0 stay 32-bit values as before
;
;  Register roles are unchanged from the original:
;    rax range / rbx src / rcx dest bit shift, ch model / rdx header
;    rsi dest / rdi window start / ebp value
;
;  in : rbx = concatenated block1+block2, pointing at the start of block2
;       rdi = destination, zero-filled and writable from -13 to length+1
;       direction flag clear, x87 initialised with two free registers
;  out: destination filled; source destroyed (it is used as scratch);
;       all registers clobbered, and xmm0/xmm1 too
; ---------------------------------------------------------------------------

%ifndef ONEKPAQ_DECOMPRESSOR_SHIFT
%define ONEKPAQ_DECOMPRESSOR_SHIFT 0
%endif

; PUSHAD saved all eight registers because that is all it could do.  Only three
; are actually live across each level, so save those:
;   context level - RAX (SBB AL,AL), RCX (CH = model), RDX (DEC in reload)
;   model level   - RAX (PMOVMSKB), RDX (CDQ makes it c0), RDI (INC per model)
; RCX survives the model level on its own: the weight loop NEGs it twice.
; RBX, RBP and RSI are only ever dereferenced inside, never written.
%macro OKP_SAVE_CONTEXT 0
    push    rax
    push    rcx
    push    rdx
%endmacro
%macro OKP_LOAD_CONTEXT 0
    pop     rdx
    pop     rcx
    pop     rax
%endmacro
%macro OKP_SAVE_MODEL 0
    push    rax
    push    rdx
    push    rdi
%endmacro
%macro OKP_LOAD_MODEL 0
    pop     rdi
    pop     rdx
    pop     rax
%endmacro

onekpaq_decompressor:
    lea     rsi, [rdi-(9+4)]            ; rsi = dest, rdi = window start
    lodsd
    inc     eax
    mov     ecx, eax

    lea     rdx, [rbx+3]                ; header = src-1 (src has a -4 offset)
                                        ; ebp is uninitialised; the loop below
                                        ; plus the first decode clean it
.normalize_loop:
    shl     byte [rbx+4], 1
    jnz     short .src_byte_has_data
    inc     rbx
    rcl     byte [rbx+4], 1             ; CF == 1
.src_byte_has_data:
    rcl     ebp, 1

.block_loop:
.byte_loop:
.bit_loop:
.normalize_start:
    add     eax, eax
    jns     short .normalize_loop

    fld1                                ; for the subrange calculation
    fld1                                ; p = 1

    OKP_SAVE_CONTEXT
    sbb     al, al                       ; was SALC

.context_loop:
    mov     ch, [rdx]

    OKP_SAVE_MODEL
    cdq
    mov     [rbx], edx                  ; c0 = c1 = -1
    movq    xmm0, [rsi]

.model_loop:
    movq    xmm1, [rdi]
    pcmpeqb xmm1, xmm0
    pmovmskb eax, xmm1
    or      al, ch
    inc     ax
    jnz     short .match_no_hit

    mov     al, [rsi+8]
    rol     al, cl
    xor     al, [rdi+8]
    shr     eax, cl
    jnz     short .match_no_hit

    dec     edx                         ; modify c1 and c0
    dec     dword [rbx]

    jc      short .match_bit_set
    sar     edx, 1
    db      0xc0                        ; turns the next instruction into
.match_bit_set:                         ; rcl cl,0x3b, i.e. a nop
    sar     dword [rbx], 1

.match_no_hit:
    inc     rdi
    cmp     rdi, rsi
    jc      short .model_loop

    ; scale the probabilities before loading them into the FPU:
    ; p *= c1/c0  =>  p = c1/(c0/p)
.weight_upload_loop:
.shift: equ $+2
    rol     dword [rbx], byte ONEKPAQ_DECOMPRESSOR_SHIFT
    fidivr  dword [rbx]
    mov     [rbx], edx
    neg     ecx
    js      short .weight_upload_loop

.model_early_start:
    OKP_LOAD_MODEL

.context_reload:
    dec     rdx
    cmp     ch, [rdx]
    jc      short .context_next
    fsqrt
    jbe     short .context_reload

.context_next:
    cmp     al, [rdx]
    jnz     short .context_loop

    OKP_LOAD_CONTEXT

    shr     eax, 1                      ; restore range

    faddp   st1                         ; subrange = range/(p+1)
    mov     [rbx], eax
    fidivr  dword [rbx]
    fistp   dword [rbx]

    sub     eax, [rbx]                  ; arithmetic decode
    cmp     ebp, eax
    jbe     .dest_bit_is_set
    inc     eax
    sub     ebp, eax
    mov     eax, [rbx]
.dest_bit_is_set:
    rcl     byte [rsi+8], 1

    ; preserves ZF where it matters, i.e. off a byte boundary
    loop    .no_full_byte
    inc     rsi
    mov     cl, 8
.no_full_byte:
    jnz     .bit_loop

onekpaq_decompressor_end:
