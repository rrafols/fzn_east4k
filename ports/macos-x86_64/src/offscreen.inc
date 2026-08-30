; ---------------------------------------------------------------------------
;  Verification build (-DOFFSCREEN=1).
;
;  Rather than capturing the display, render into a framebuffer object and dump
;  a fixed set of frames as PPM plus the mixed tune as a WAV.  This is how the
;  port was checked against the original, and how the packed build is checked
;  against the linked one, without taking over the screen.
; ---------------------------------------------------------------------------

%define SHOT_W RENDER_W
%define SHOT_H RENDER_H
%define SHOT_COUNT 13

init_offscreen:
    push    r12
    lea     rdi, [pfattr_off]
    lea     rsi, [pixfmt]
    lea     rdx, [npix]
    CALLI   CGLChoosePixelFormat
    mov     rdi, [pixfmt]
    xor     esi, esi
    lea     rdx, [ctx]
    CALLI   CGLCreateContext
    mov     rdi, [ctx]
    CALLI   CGLSetCurrentContext

    ; There is no drawable here, so unlike the real build this needs a genuine
    ; framebuffer object to render into.  The scene itself is identical.
    mov     edi, 1
    lea     rsi, [fbo]
    CALLI   glGenFramebuffersEXT
    mov     edi, GL_FRAMEBUFFER_EXT
    mov     esi, [fbo]
    CALLI   glBindFramebufferEXT

    mov     edi, 1
    lea     rsi, [rbo]
    CALLI   glGenRenderbuffersEXT
    mov     edi, GL_RENDERBUFFER_EXT
    mov     esi, [rbo]
    CALLI   glBindRenderbufferEXT
    mov     edi, GL_RENDERBUFFER_EXT
    mov     esi, GL_RGBA8
    mov     edx, RENDER_W
    mov     ecx, RENDER_H
    CALLI   glRenderbufferStorageEXT
    mov     edi, GL_FRAMEBUFFER_EXT
    mov     esi, GL_COLOR_ATTACHMENT0
    mov     edx, GL_RENDERBUFFER_EXT
    mov     ecx, [rbo]
    CALLI   glFramebufferRenderbufferEXT

    mov     edi, 1
    lea     rsi, [dbo]
    CALLI   glGenRenderbuffersEXT
    mov     edi, GL_RENDERBUFFER_EXT
    mov     esi, [dbo]
    CALLI   glBindRenderbufferEXT
    mov     edi, GL_RENDERBUFFER_EXT
    mov     esi, GL_DEPTH_COMPONENT24
    mov     edx, RENDER_W
    mov     ecx, RENDER_H
    CALLI   glRenderbufferStorageEXT
    mov     edi, GL_FRAMEBUFFER_EXT
    mov     esi, GL_DEPTH_ATTACHMENT
    mov     edx, GL_RENDERBUFFER_EXT
    mov     ecx, [dbo]
    CALLI   glFramebufferRenderbufferEXT

    xor     edi, edi
    xor     esi, esi
    mov     edx, RENDER_W
    mov     ecx, RENDER_H
    CALLI   glViewport
    pop     r12
    ret

; Frames are picked from a table instead of the clock, so runs are comparable.
get_ms:
    mov     eax, [shotidx]
    lea     rsi, [shot_ms]
    mov     eax, [rsi + rax*4]
    ret

grab_frame:
    push    r12
    xor     edi, edi
    xor     esi, esi
    mov     edx, SHOT_W
    mov     ecx, SHOT_H
    mov     r8d, GL_RGB
    mov     r9d, GL_UNSIGNED_BYTE
    lea     rax, [shotbuf]
    push    rax                         ; seventh argument
    push    rax                         ; and keep RSP aligned
    CALLI   glReadPixels
    add     rsp, 16

    mov     eax, [shotidx]              ; name the file shotNN.ppm
    mov     ecx, eax
    shr     ecx, 4
    add     ecx, '0'
    mov     [shotname+4], cl
    and     eax, 15
    cmp     eax, 9
    jle     .digit
    add     eax, 'a' - '0' - 10
.digit:
    add     eax, '0'
    mov     [shotname+5], al

    lea     rdi, [shotname]
    lea     rsi, [wbmode]
    CALLI   fopen
    mov     r12, rax

    lea     rdi, [ppmhdr]
    mov     esi, 1
    mov     edx, ppmhdr_len
    mov     rcx, r12
    CALLI   fwrite

    mov     r13d, SHOT_H - 1            ; glReadPixels hands back bottom-up
.row:
    imul    eax, r13d, SHOT_W*3
    lea     rdi, [shotbuf]
    add     rdi, rax
    mov     esi, 1
    mov     edx, SHOT_W*3
    mov     rcx, r12
    CALLI   fwrite
    dec     r13d
    jns     .row

    mov     rdi, r12
    CALLI   fclose

    inc     dword [shotidx]
    mov     eax, [shotidx]
    cmp     eax, SHOT_COUNT
    jae     exitIntro
    pop     r12
    ret

dump_wav:
    push    r12
    lea     rdi, [wavname]
    lea     rsi, [wbmode]
    CALLI   fopen
    mov     r12, rax
    lea     rdi, [wavhdr]
    mov     esi, 1
    mov     edx, 44
    mov     rcx, r12
    CALLI   fwrite
    lea     rdi, [pcmbuf]
    mov     esi, 1
    mov     edx, TOTAL_SAMPLES*2
    mov     rcx, r12
    CALLI   fwrite
    mov     rdi, r12
    CALLI   fclose
    pop     r12
    ret

section .rodata
align 8
pfattr_off: dd kCGLPFAAccelerated, kCGLPFAColorSize, 24, kCGLPFADepthSize, 32, 0
wbmode:     db "wb", 0
wavname:    db "song.wav", 0
ppmhdr:     db "P6", 10
            db %str(SHOT_W), " ", %str(SHOT_H), 10, "255", 10
ppmhdr_len  equ $ - ppmhdr

; 44-byte canonical WAV header, mono 16-bit at 44100 Hz
wavhdr:     db "RIFF"
            dd 36 + TOTAL_SAMPLES*2
            db "WAVEfmt "
            dd 16
            dw 1, 1
            dd SND_RATE, SND_RATE*2
            dw 2, 16
            db "data"
            dd TOTAL_SAMPLES*2

; One frame per interesting orderStuff combination.
shot_ms:
            dd  900 +  0*1824           ; order 00  effect 6
            dd  900 +  8*1824           ; order 08  effects 3,6
            dd  900 + 16*1824           ; order 16  effect 2
            dd  900 + 20*1824           ; order 20  effects 2,3
            dd  900 + 24*1824           ; order 24  effect 1
            dd  900 + 28*1824           ; order 28  effects 1,2
            dd  900 + 32*1824           ; order 32  effects 1,2,7
            dd  900 + 36*1824           ; order 36  effects 2,4
            dd  900 + 40*1824           ; order 40  effects 3,4
            dd  900 + 48*1824           ; order 48  effects 1,4
            dd  900 + 49*1824           ; order 49  effects 2,5
            dd  900 + 52*1824           ; order 52  effects 1,5
            dd  900 +  4*1824           ; order 04  effects 6,7,8

section .data
shotname:   db "shot00.ppm", 0

section .bss
alignb 16
shotbuf:    resb SHOT_W * SHOT_H * 3
shotidx:    resd 1
fbo:        resd 1
rbo:        resd 1
dbo:        resd 1
