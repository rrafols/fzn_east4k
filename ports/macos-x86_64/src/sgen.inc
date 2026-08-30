; ---------------------------------------------------------------------------
;  Instrument synthesis - a direct port of the original sgen.inc.
;
;  Each of the eight instruments is an oscillator (sine, saw, or square,
;  optionally rectified) run through a resonant low-pass, written out as
;  unsigned 8-bit.  The x87 code is unchanged; only the addressing moved to
;  64-bit.  The parameter tables are contiguous in samples.inc, so a single
;  base register reaches all six of them.
; ---------------------------------------------------------------------------

; in: esi = instrument index (0..NSAMPLES-1)
gensample:
    lea     r8, [m_Freq]
    lea     r8, [r8 + rsi*4]            ; +0 freq, +32 dfreq, +64 amp,
                                        ; +96 damp, +128 c, +160 r
    lea     r9, [m_nBytes]
    movzx   ecx, word [r9 + rsi*2]      ; length in bytes
    lea     r9, [m_tLfo]
    add     r9, rsi                     ; -> waveform flags

    lea     rdi, [sampbuf]
    imul    r10, rsi, 20000
    add     rdi, r10

    finit
    fldz                                ; sp
    fldz                                ; ps    sp
    fldz                                ; phase ps    sp
    fld     dword [r8+64]               ; amp   phase ps    sp
    fld     dword [r8]                  ; freq  amp   phase ps sp
.loop:
    fadd    st2, st0                    ; phase += freq
    fadd    dword [r8+32]               ; freq  += dfreq
    fld     st2                         ; val   freq  amp   phase ps sp

    mov     al, [r9]
    test    al, 1
    je      .saw
    fsin
    jmp     .nosaw
.saw:                                   ; wrap the ramp through a signed byte
    fistp   word [wTemp]
    mov     dl, byte [wTemp]
    movsx   dx, dl
    mov     [wTemp], dx
    fild    word [wTemp]
.nosaw:
    test    al, 2
    je      .nosquare
    fistp   word [wTemp]
    and     word [wTemp], 0x80
    fild    word [wTemp]
.nosquare:
    test    al, 4
    je      .nofabs
    fabs
.nofabs:
    test    al, 8
    je      .nopos
    fld     st0
    fabs
    faddp   st1, st0
.nopos:
    fmul    st0, st2                    ; val *= amp
    fxch    st2
    fadd    dword [r8+96]               ; amp += damp
    fxch    st2

    fsub    st0, st4                    ; val - ps
    fmul    dword [r8+128]              ; * c
    faddp   st5, st0                    ; sp += (val-ps)*c
    fld     st4                         ; sp    freq amp phase ps sp
    fadd    st4, st0                    ; ps += sp
    fxch    st4
    fist    word [wTemp]                ; output sample = ps
    fxch    st4
    mov     ax, word [wTemp]
    fmul    dword [r8+160]              ; sp *= r
    fstp    st5

    add     al, 127
    stosb
    dec     ecx
    jnz     .loop

    finit                               ; leave the x87 stack empty for callers
    ret
