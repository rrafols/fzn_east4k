; ---------------------------------------------------------------------------
;  Module playback.
;
;  The original handed eight DirectSound secondary buffers to the mixer and
;  drove them from a thread that woke every 114 ms.  CoreAudio has no such
;  per-buffer pitch/volume, so the equivalent mixing happens here: the whole
;  tune is rendered once into a single AudioQueue buffer before the demo
;  starts, which also removes the player thread and its synchronisation.
;
;  A voice is 16 bytes: sample index (-1 = idle), position, step, gain.
; ---------------------------------------------------------------------------

; in: rdi = 16-bit PCM destination (TOTAL_SAMPLES entries)
rendersong:
    push    r12
    push    r12
    push    r13
    push    r14
    push    r15

    lea     rsi, [voices]               ; all voices idle
    mov     ecx, NCHANNELS
.silence:
    mov     dword [rsi], -1
    mov     dword [rsi+4], 0
    mov     dword [rsi+8], 0
    mov     dword [rsi+12], 0
    add     rsi, 16
    dec     ecx
    jnz     .silence

    xor     r12d, r12d                  ; order
.order:
    lea     rax, [orderList]
    movzx   eax, byte [rax + r12]
    shl     eax, 3                      ; eight channels per pattern
    mov     r13d, eax
    xor     r14d, r14d                  ; row
.row:
    xor     ecx, ecx                    ; channel
.chan:
    lea     rsi, [patternList]
    add     rsi, r13
    movsx   eax, byte [rsi + rcx]
    test    eax, eax
    js      .nextchan                   ; 255 = channel unused in this pattern

    lea     rsi, [channelList]
    imul    eax, eax, 17                ; one sample byte + 16 rows
    add     rsi, rax
    movzx   r8d, byte [rsi]             ; instrument
    add     rsi, r14
    movzx   eax, byte [rsi + 1]         ; note byte for this row
    test    eax, eax
    jz      .nextchan

    mov     r9, rcx                     ; -> voice
    shl     r9, 4
    lea     r10, [voices]
    add     r9, r10

    mov     edx, eax
    and     edx, 15
    dec     edx                         ; -1 = keep pitch, 14 = note cut
    mov     r11d, eax
    shr     r11d, 6
    and     r11d, 3
    lea     r10, [volgain]
    mov     r10d, [r10 + r11*4]         ; gain for this note's volume column

    cmp     edx, 14
    je      .cut
    cmp     edx, -1
    je      .volonly

    shr     eax, 4                      ; octave
    and     eax, 3
    add     eax, 4
    imul    eax, eax, 12
    add     eax, edx                    ; note index into the frequency table
    mov     [r9], r8d
    mov     dword [r9+4], 0
    lea     r11, [freqtable]
    cvtsi2ss xmm0, dword [r11 + rax*4]
    divss   xmm0, [rate_f]
    movss   [r9+8], xmm0
    mov     [r9+12], r10d
    jmp     .nextchan
.volonly:
    mov     [r9+12], r10d
    jmp     .nextchan
.cut:
    mov     dword [r9], -1
    mov     dword [r9+4], 0
    mov     dword [r9+12], 0
.nextchan:
    inc     ecx
    cmp     ecx, NCHANNELS
    jne     .chan

    ; ---- mix this row -----------------------------------------------------
    mov     r15d, ROW_SAMPLES
.sample:
    xorps   xmm7, xmm7
    lea     r9, [voices]
    mov     ecx, NCHANNELS
.mix:
    mov     eax, [r9]
    test    eax, eax
    js      .nextvoice
    cvttss2si edx, dword [r9+4]
    lea     r10, [m_nBytes]
    movzx   r11d, word [r10 + rax*2]
    cmp     edx, r11d
    jl      .inrange
    mov     dword [r9], -1              ; one-shot finished
    jmp     .nextvoice
.inrange:
    imul    r10d, eax, 20000
    lea     r11, [sampbuf]
    add     r11, r10
    movzx   r10d, byte [r11 + rdx]
    sub     r10d, 128                   ; unsigned 8-bit -> signed
    cvtsi2ss xmm0, r10d
    mulss   xmm0, [r9+12]
    addss   xmm7, xmm0
    movss   xmm0, [r9+8]
    addss   xmm0, [r9+4]
    movss   [r9+4], xmm0
.nextvoice:
    add     r9, 16
    dec     ecx
    jnz     .mix

    mulss   xmm7, [mastergain]
    cvtps2dq xmm7, xmm7                 ; the upper lanes stay zero
    packssdw xmm7, xmm7                 ; saturating -> 16 bit
    movd    eax, xmm7
    mov     [rdi], ax
    add     rdi, 2
    dec     r15d
    jnz     .sample

    inc     r14d
    cmp     r14d, ROWS
    jne     .row
    inc     r12d
    cmp     r12d, ORDERS
    jne     .order

    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r12
    ret
