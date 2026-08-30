; ---------------------------------------------------------------------------
;  Calling helpers.
;
;  The Win32 original passed everything on the stack (stdcall) and reached the
;  imports through EBP.  System V AMD64 passes integers in RDI/RSI/RDX/RCX/R8/R9
;  and floats in XMM0-7, so every INVOKER site becomes a few register loads
;  followed by an indirect call through the import table.
;
;  RBX holds "imports + 128" for the whole run, which puts the 32 hottest
;  entries within a signed-byte displacement (3-byte call instead of 6).
; ---------------------------------------------------------------------------

; Assert 16-byte stack alignment at every call site (debug builds only).
%macro CHKA 0
  %if DEBUG
    test    rsp, 15
    jz      %%aligned
    int3
  %%aligned:
  %endif
%endmacro

%macro CALLI 1                          ; call an imported function
    CHKA
    call    qword [rbx + O_%1]
%endmacro

; -- float argument loading -------------------------------------------------
%macro FZ 1                             ; xmm%1 = 0.0f
    xorps   xmm%1, xmm%1
%endmacro

%macro FK 2                             ; xmm%1 = float constant %2 (pool label)
    movss   xmm%1, [%2]
%endmacro

%macro FM 2                             ; xmm%1 = float in memory operand %2
    movss   xmm%1, %2
%endmacro

%macro FR 2                             ; xmm%1 = float bit pattern in GPR %2
    movd    xmm%1, %2
%endmacro

; xmm%1 = st0, popped.  The staging slots live in the RBP pool, so each of
; these assembles to the same eight bytes wherever it appears.
%macro FPOP 1
    fstp    dword [fp %+ %1]
    movss   xmm%1, [fp %+ %1]
%endmacro

; -- callee-saved scratch ---------------------------------------------------
; The Win32 code leaned on PUSHAD because the Win32 API preserves EBX/ESI/EDI.
; Here only RBX/RBP/R12-R15 survive a call, so loop state lives in those and
; these two macros stand in for PUSHAD/POPAD.  Four pushes keeps RSP aligned.
%macro PUSHA 0
    push    r12
    push    r13
    push    r14
    push    r15
%endmacro

%macro POPA 0
    pop     r15
    pop     r14
    pop     r13
    pop     r12
%endmacro

; -- import table -----------------------------------------------------------
; The list is expanded twice: once to define O_<name> offsets, once to emit the
; table itself.  Both builds share the layout, so the call sites are identical
; whether the table is filled by the linker or by our own binder.
%macro DECLARE_IMPORT_OFFSETS 0
    %assign IMPN 0
    %macro IMP 1
        %xdefine O_%1 (IMPN * 8 - 128)
        %assign IMPN IMPN + 1
    %endmacro
    IMPORT_LIST
    %unmacro IMP 1
    %assign IMPORT_COUNT IMPN
%endmacro

%macro EMIT_IMPORT_TABLE 0
    %macro IMP 1
        extern  _%1
        dq      _%1
    %endmacro
imports:
    IMPORT_LIST
    %unmacro IMP 1
%endmacro

; The packed build has no linker to fill the table in, so the names are kept
; as one NUL-separated blob and resolved with dlsym at start-up.
%macro EMIT_IMPORT_NAMES 0
    %macro IMP 1
        %defstr IMPSTR %1
        db      IMPSTR, 0
    %endmacro
symnames:
    IMPORT_LIST
    db      0
    %unmacro IMP 1
%endmacro
