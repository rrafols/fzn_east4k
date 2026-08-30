; ===========================================================================
;  "Looking for the East" - Fuzzion, 2003
;  macOS / x86-64 port of the original Win32 4k intro.
;
;  The effect code is the same x87 arithmetic as the original; what changed is
;  the platform underneath it:
;
;      CreateWindowEx + wgl   ->  CGL fullscreen on the captured main display
;      DirectSound            ->  one pre-rendered AudioQueue buffer
;      stdcall (stack args)   ->  System V AMD64 (registers)
;      GetTickCount + thread  ->  a single monotonic clock
; ===========================================================================

bits 64
default rel

%ifndef DEBUG
  %define DEBUG 0
%endif
%ifndef OFFSCREEN
  %define OFFSCREEN 0
%endif
%ifndef TINY
  %define TINY 0
%endif
%ifndef WINDOWED
  %define WINDOWED 0
%endif

; The packed build is one flat RWX blob, so everything shares .text and only
; the zero-filled tail stays in .bss.
%if TINY
  %define S_RODATA .text
  %define S_DATA   .text
%else
  %define S_RODATA .rodata
  %define S_DATA   .data
%endif

%include "consts.inc"
%include "macros.inc"
%include "imports.inc"
%include "data.inc"

DECLARE_IMPORT_OFFSETS

%define CLOCK_UPTIME_RAW 8

%if TINY
[map symbols build/payload.map]
%endif

section .text
%if TINY
; The stub hands over dlsym in RDI and dlopen in RSI.
_start:
    and     rsp, -16
    mov     r13, rdi                    ; dlsym
    mov     r12, rsi                    ; dlopen

    lea     r15, [dylibs]               ; make the frameworks searchable
.opendylib:
    cmp     byte [r15], 0
    je      .dylibsdone
    mov     rdi, r15
    mov     esi, 1                      ; RTLD_LAZY
    call    r12
.skipdylib:
    mov     al, [r15]
    inc     r15
    test    al, al
    jnz     .skipdylib
    jmp     .opendylib

.dylibsdone:
    lea     r14, [imports]
    lea     r15, [symnames]
.resolve:
    cmp     byte [r15], 0
    je      .resolved
    mov     rdi, -2
    mov     rsi, r15
    call    r13
    mov     [r14], rax
    add     r14, 8
.skipname:
    mov     al, [r15]
    inc     r15
    test    al, al
    jnz     .skipname
    jmp     .resolve
.resolved:
    lea     rbx, [imports + 128]
    lea     rbp, [kbase + 128]
%else
global _main
_main:
    and     rsp, -16                    ; SysV wants 16-byte alignment at calls
    lea     rbx, [imports + 128]        ; hot imports reach via disp8
    lea     rbp, [kbase + 128]          ; constants reach via disp8
%endif

    call    unpackcube

; ---------------------------------------------------------------------------
;  Display and OpenGL context
; ---------------------------------------------------------------------------
%if OFFSCREEN
    call    init_offscreen
%elif WINDOWED
    call    init_window
%else
    CALLI   CGMainDisplayID
    mov     [dispid], eax
    mov     edi, eax
    CALLI   CGDisplayCapture

    mov     edi, [dispid]
    CALLI   CGDisplayIDToOpenGLDisplayMask
    mov     [dmask], eax

    lea     rdi, [pfattr]
    lea     rsi, [pixfmt]
    lea     rdx, [npix]
    CALLI   CGLChoosePixelFormat

    mov     rdi, [pixfmt]
    xor     esi, esi
    lea     rdx, [ctx]
    CALLI   CGLCreateContext

    mov     rdi, [ctx]
    CALLI   CGLSetCurrentContext
    mov     rdi, [ctx]
    mov     esi, [dmask]
    CALLI   CGLSetFullScreenOnDisplay

    mov     rdi, [ctx]                  ; wait for vblank on flush
    mov     esi, kCGLCPSwapInterval
    lea     rdx, [one_i]
    CALLI   CGLSetParameter

    ; The original ran at a forced 640x480 and its look depends on that:
    ; glLineWidth and glPointSize are in pixels, and the driver silently clamps
    ; smooth points to 64, so a scaled-up logo dot never draws at the right
    ; size.  So render into a 640x480 framebuffer exactly as the original did,
    ; and blit that to the display.  Every pixel-unit primitive is then right.
    ; Ask GL for the drawable it actually gave us rather than asking
    ; CoreGraphics for the display size - on a scaled HiDPI mode those are not
    ; the same number, and the viewport is what the image has to land in.
    mov     edi, GL_VIEWPORT
    lea     rsi, [vpbuf]
    CALLI   glGetIntegerv
    mov     r14d, [vpbuf+8]
    mov     r15d, [vpbuf+12]

    lea     rax, [r15*4]                ; the 4:3 box to blit into
    xor     edx, edx
    mov     ecx, 3
    div     rcx
    mov     r13, r15
    cmp     rax, r14
    jbe     .box
    mov     rax, r14
    lea     r13, [r14*2]
    add     r13, r14
    shr     r13, 2
.box:
    mov     [blitw], eax
    mov     [blith], r13d
    sub     r14, rax
    shr     r14, 1
    mov     [blitx], r14d
    sub     r15, r13
    shr     r15, 1
    mov     [blity], r15d

    call    make_tex
%endif

; ---------------------------------------------------------------------------
;  Instruments and the note frequency table
; ---------------------------------------------------------------------------
    xor     esi, esi
.gensamples:
    push    rsi
    call    gensample
    pop     rsi
    inc     esi
    cmp     esi, NSAMPLES
    jne     .gensamples

    fld     dword [c33152]
    lea     rdi, [freqtable]
    mov     ecx, 8*12
.genfreq:
    fld     st0
    fdiv    dword [cDiv]
    fistp   dword [rdi]
    add     rdi, 4
    fmul    dword [c105]
    dec     ecx
    jnz     .genfreq
    fstp    st0

; ---------------------------------------------------------------------------
;  Lens-flare texture: two gaussians, one wide and one tight, combined into
;  a slightly different curve per channel.
; ---------------------------------------------------------------------------
    lea     rdi, [flare]
    mov     edx, FLARE_DIM
.flare_i:
    fild    word [i64]                  ; column coordinate, counts down
    mov     ecx, FLARE_DIM
.flare_j:
    mov     [wTemp], edx
    fild    dword [wTemp]
    fisub   word [i64]
    fmul    st0, st0                    ; (i-64)^2
    fld     st1
    fmul    st0, st0                    ; (j-64)^2
    faddp   st1, st0
    fsqrt                               ; f = distance from the centre

    fld     st0
    fchs
    fld     st0
    fmul    st0, st2
    fmul    dword [f0_005]
    call    fexp
    fmul    dword [f0_6]                ; ff  = 0.6 * exp(-0.005 f*f)

    fxch    st2
    fmulp   st1, st0
    fmul    dword [f0_8]
    call    fexp
    fmul    dword [f0_4]                ; ssi = 0.4 * exp(-0.8 f*f)

    fld     st1
    fmul    dword [f0_7]
    fadd    st0, st1
    fstp    dword [rdi+4]               ; G
    fld     st1
    fadd    st0, st0
    fadd    st0, st1
    fstp    dword [rdi]                 ; R
    fld     st1
    fmul    dword [f0_25]
    fadd    st0, st1
    fstp    dword [rdi+8]               ; B
    faddp   st1, st0
    fstp    dword [rdi+12]              ; A

    add     rdi, 16
    fld1
    fsubp   st1, st0
    dec     ecx
    jnz     .flare_j
    fstp    st0
    dec     edx
    jnz     .flare_i

; ---------------------------------------------------------------------------
;  Particles.  holdrand is still zero here, exactly as in the original, so
;  the particle field is identical from run to run.
; ---------------------------------------------------------------------------
    lea     rdi, [px]
    mov     ecx, MAX_PARTICLES
.partPrecalc:
    fild    word [i40]
    call    frand
    fisub   word [i40]
    fstp    dword [rdi]                 ; position
    fld1
    call    frand
    fadd    dword [f0_5]
    fidiv   word [i4]
    fstp    dword [rdi + MAX_PARTICLES*4]   ; velocity
    add     rdi, 4
    dec     ecx
    jnz     .partPrecalc

; ---------------------------------------------------------------------------
;  Upload the flare texture and compile the billboard quad into list 1.
; ---------------------------------------------------------------------------
    mov     edi, GL_TEXTURE_2D
    mov     esi, GL_TEXTURE_MIN_FILTER
    mov     edx, GL_LINEAR
    CALLI   glTexParameteri

    ; glTexImage2D(target, level, internalformat, w, h, border, format, type,
    ; pixels) - the first six go in registers, the last three on the stack.
    lea     rax, [flare]
    sub     rsp, 32
    mov     qword [rsp], GL_RGBA
    mov     qword [rsp+8], GL_FLOAT
    mov     [rsp+16], rax
    mov     edi, GL_TEXTURE_2D
    xor     esi, esi
    mov     edx, 4
    mov     ecx, FLARE_DIM
    mov     r8d, FLARE_DIM
    xor     r9d, r9d
    CALLI   glTexImage2D
    add     rsp, 32

    mov     edi, 1
    mov     esi, GL_COMPILE
    CALLI   glNewList
    mov     edi, GL_QUADS
    CALLI   glBegin

    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    CALLI   glTexCoord2f
    movss   xmm0, [fm2_0]
    movss   xmm1, [fm2_0]
    xorps   xmm2, xmm2
    CALLI   glVertex3f

    movss   xmm0, [f1_0]
    xorps   xmm1, xmm1
    CALLI   glTexCoord2f
    movss   xmm0, [f2_0]
    movss   xmm1, [fm2_0]
    xorps   xmm2, xmm2
    CALLI   glVertex3f

    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    CALLI   glTexCoord2f
    movss   xmm0, [f2_0]
    movss   xmm1, [f2_0]
    xorps   xmm2, xmm2
    CALLI   glVertex3f

    xorps   xmm0, xmm0
    movss   xmm1, [f1_0]
    CALLI   glTexCoord2f
    movss   xmm0, [fm2_0]
    movss   xmm1, [f2_0]
    xorps   xmm2, xmm2
    CALLI   glVertex3f

    CALLI   glEnd
    CALLI   glEndList

; ---------------------------------------------------------------------------
;  Audio: render the tune into one queue buffer and start it.
; ---------------------------------------------------------------------------
%if OFFSCREEN
    lea     rdi, [pcmbuf]
    call    rendersong
    call    dump_wav
%else
    movsd   xmm0, [d_rate]
    movsd   [asbd], xmm0
    lea     rdi, [asbd]
    lea     rsi, [aq_callback]
    xor     edx, edx
    xor     ecx, ecx
    xor     r8d, r8d
    xor     r9d, r9d
    lea     rax, [aq]
    push    rax                         ; 7th argument: &queue
    push    rax                         ; also keeps RSP aligned
    CALLI   AudioQueueNewOutput
    add     rsp, 16

    mov     rdi, [aq]
    mov     esi, TOTAL_SAMPLES * 2
    lea     rdx, [aqbuf]
    CALLI   AudioQueueAllocateBuffer

    mov     rax, [aqbuf]
    mov     dword [rax+16], TOTAL_SAMPLES * 2
    mov     rdi, [rax+8]
    call    rendersong

    mov     rdi, [aq]
    mov     rsi, [aqbuf]
    xor     edx, edx
    xor     ecx, ecx
    CALLI   AudioQueueEnqueueBuffer
    mov     rdi, [aq]
    xor     esi, esi
    CALLI   AudioQueueStart
%endif

; ---------------------------------------------------------------------------
;  Start the clock.  The original re-seeded its PRNG from GetTickCount at this
;  point, so the background lines differ between runs; keep that.
; ---------------------------------------------------------------------------
    mov     edi, CLOCK_UPTIME_RAW
    CALLI   clock_gettime_nsec_np
    mov     [t_start], rax
    mov     [holdrand], eax

%include "frame.inc"

; ---------------------------------------------------------------------------
exitIntro:
%if OFFSCREEN || WINDOWED
    xor     edi, edi
    CALLI   exit
%else
    mov     edi, [dispid]
    CALLI   CGDisplayRelease
    xor     edi, edi
    CALLI   exit
%endif

; AudioQueue insists on a callback; the buffer is never recycled, so it is
; only ever reached when playback of the whole tune finishes.
aq_callback:
    ret

%if OFFSCREEN == 0
; Milliseconds since the demo started.
get_ms:
    push    rax
    mov     edi, CLOCK_UPTIME_RAW
    CALLI   clock_gettime_nsec_np
    sub     rax, [t_start]
    xor     edx, edx
    mov     ecx, 1000000
    div     rcx
    pop     rdx
    ret
%endif

%include "draw.inc"
%include "sgen.inc"
%include "player.inc"
%if OFFSCREEN
  %include "offscreen.inc"
%endif
%if WINDOWED
  %include "window.inc"
%endif

; ---------------------------------------------------------------------------
;  Data
; ---------------------------------------------------------------------------
section S_RODATA
align 4
%include "cubedata.inc"
%include "chin_char.inc"
%include "samples.inc"
%include "zik.asm"
%include "orderstuff.inc"

%if OFFSCREEN
section .bss
alignb 16
pcmbuf:     resb TOTAL_SAMPLES * 2
%endif

section .bss
alignb 16
bss_end:                                ; the packer sizes the segment from this

section S_DATA
align 8
%if TINY
    EMIT_IMPORT_NAMES
dylibs:
    db "/System/Library/Frameworks/OpenGL.framework/OpenGL", 0
    db "/System/Library/Frameworks/ApplicationServices.framework/ApplicationServices", 0
    db "/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox", 0
    db 0
section .bss
alignb 8
imports:    resq IMPORT_COUNT
%else
    EMIT_IMPORT_TABLE
%endif
