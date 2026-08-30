; ---------------------------------------------------------------------------
;  Windowed mode (-DWINDOWED=1).
;
;  CGL can capture a display on its own, but it cannot make a window, so this
;  goes through Cocoa: NSApplication, NSWindow and NSOpenGLContext, driven from
;  assembly with objc_msgSend.  That costs roughly 700 bytes, which is why it
;  is a separate build and not part of the packed 4k intro.
; ---------------------------------------------------------------------------

%define WIN_W 1280
%define WIN_H 960                       ; 4:3, the aspect the intro was made for
%define NSWindowStyleMaskTitled     1
%define NSWindowStyleMaskClosable   2
%define NSBackingStoreBuffered      2
%define NSOpenGLPFADoubleBuffer     5
%define NSOpenGLPFAColorSize        8
%define NSOpenGLPFADepthSize       12
%define NSOpenGLPFAAccelerated     73

; rdi = object, rsi = selector index; remaining arguments already in place
%macro MSG 2
    mov     rdi, %1
    lea     rax, [sels]
    mov     rsi, [rax + %2 * 8]
    xor     eax, eax
    CALLI   objc_msgSend
%endmacro

%macro CLS 1
    lea     rdi, [%1]
    CALLI   objc_getClass
%endmacro

%define SEL_sharedApplication   0
%define SEL_setActivationPolicy 1
%define SEL_alloc               2
%define SEL_initWindow          3
%define SEL_initWithAttributes  4
%define SEL_initWithFormat      5
%define SEL_contentView         6
%define SEL_setView             7
%define SEL_makeCurrentContext  8
%define SEL_makeKeyAndOrderFront 9
%define SEL_activateIgnoring    10
%define SEL_finishLaunching     11
%define SEL_nextEvent           12
%define SEL_sendEvent           13
%define SEL_flushBuffer         14
%define SEL_setTitle            15
%define SEL_stringWithUTF8      16
%define SEL_update              17

init_window:
    push    r12
    push    r13
    push    r14

    lea     r15, [selnames]             ; register every selector once
    lea     r14, [sels]
.regsel:
    cmp     byte [r15], 0
    je      .selsdone
    mov     rdi, r15
    CALLI   sel_registerName
    mov     [r14], rax
    add     r14, 8
.skipsel:
    mov     al, [r15]
    inc     r15
    test    al, al
    jnz     .skipsel
    jmp     .regsel
.selsdone:

    CLS     s_NSApplication
    MSG     rax, SEL_sharedApplication
    mov     r13, rax                    ; the shared application
    xor     edx, edx                    ; NSApplicationActivationPolicyRegular
    MSG     r13, SEL_setActivationPolicy

    CLS     s_NSWindow
    MSG     rax, SEL_alloc
    ; initWithContentRect:styleMask:backing:defer: - NSRect is four doubles and
    ; larger than sixteen bytes, so it travels on the stack.
    sub     rsp, 32
    xorps   xmm0, xmm0
    movsd   [rsp], xmm0
    movsd   [rsp+8], xmm0
    movsd   xmm0, [d_winw]
    movsd   [rsp+16], xmm0
    movsd   xmm0, [d_winh]
    movsd   [rsp+24], xmm0
    mov     rdi, rax
    lea     rcx, [sels]
    mov     rsi, [rcx + SEL_initWindow * 8]
    mov     edx, NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
    mov     ecx, NSBackingStoreBuffered
    xor     r8d, r8d
    xor     eax, eax
    CALLI   objc_msgSend
    add     rsp, 32
    mov     r12, rax                    ; the window

    CLS     s_NSOpenGLPixelFormat
    MSG     rax, SEL_alloc
    lea     rdx, [nsattr]
    MSG     rax, SEL_initWithAttributes
    mov     r14, rax

    CLS     s_NSOpenGLContext
    MSG     rax, SEL_alloc
    mov     rdx, r14
    xor     ecx, ecx
    MSG     rax, SEL_initWithFormat
    mov     [ctx], rax

    CLS     s_NSString                  ; window title
    lea     rdx, [s_title]
    MSG     rax, SEL_stringWithUTF8
    mov     rdx, rax
    MSG     r12, SEL_setTitle

    ; The window has to be on screen before the context is given its view,
    ; otherwise the context ends up with no drawable and everything drawn to
    ; framebuffer 0 goes nowhere.
    xor     edx, edx
    MSG     r12, SEL_makeKeyAndOrderFront
    mov     edx, 1
    MSG     r13, SEL_activateIgnoring
    MSG     r13, SEL_finishLaunching
    mov     [nsapp], r13

    MSG     r12, SEL_contentView
    mov     rdx, rax
    MSG     qword [ctx], SEL_setView
    MSG     qword [ctx], SEL_update      ; bind the context to the new drawable
    MSG     qword [ctx], SEL_makeCurrentContext

    mov     dword [blitx], 0            ; the window is already 4:3
    mov     dword [blity], 0
    mov     dword [blitw], WIN_W
    mov     dword [blith], WIN_H
    call    make_tex

    pop     r14
    pop     r13
    pop     r12
    ret

; Drain the event queue, then show the frame.
window_flush:
    push    r12
.pump:
    mov     rdi, [nsapp]
    lea     rax, [sels]
    mov     rsi, [rax + SEL_nextEvent * 8]
    mov     rdx, -1                     ; NSEventMaskAny
    xor     ecx, ecx                    ; untilDate: nil - do not block
    mov     r8, [rbx + O_NSDefaultRunLoopMode]   ; the symbol is the variable,
    mov     r8, [r8]                            ; the mode is what it holds
    mov     r9d, 1                      ; dequeue
    xor     eax, eax
    CALLI   objc_msgSend
    test    rax, rax
    jz      .drained
    mov     rdx, rax
    MSG     qword [nsapp], SEL_sendEvent
    jmp     .pump
.drained:
    MSG     qword [ctx], SEL_flushBuffer
    pop     r12
    ret

section .rodata
align 8
s_NSApplication:      db "NSApplication", 0
s_NSWindow:           db "NSWindow", 0
s_NSOpenGLPixelFormat:db "NSOpenGLPixelFormat", 0
s_NSOpenGLContext:    db "NSOpenGLContext", 0
s_NSString:           db "NSString", 0
s_title:              db "looking for the east", 0
selnames:
    db "sharedApplication", 0
    db "setActivationPolicy:", 0
    db "alloc", 0
    db "initWithContentRect:styleMask:backing:defer:", 0
    db "initWithAttributes:", 0
    db "initWithFormat:shareContext:", 0
    db "contentView", 0
    db "setView:", 0
    db "makeCurrentContext", 0
    db "makeKeyAndOrderFront:", 0
    db "activateIgnoringOtherApps:", 0
    db "finishLaunching", 0
    db "nextEventMatchingMask:untilDate:inMode:dequeue:", 0
    db "sendEvent:", 0
    db "flushBuffer", 0
    db "setTitle:", 0
    db "stringWithUTF8String:", 0
    db "update", 0
    db 0
align 4
nsattr: dd NSOpenGLPFAAccelerated, NSOpenGLPFADoubleBuffer
        dd NSOpenGLPFAColorSize, 24, NSOpenGLPFADepthSize, 32, 0

section .data
align 8
d_winw: dq 1280.0               ; NSRect wants doubles, not integers
d_winh: dq 960.0

section .bss
alignb 8
sels:   resq 18
nsapp:  resq 1
