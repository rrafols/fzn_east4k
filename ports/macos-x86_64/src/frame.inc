; ===========================================================================
;  Per-frame code.
;
;  The original ran the visuals off GetTickCount while a separate thread
;  stepped the module; both are derived from one monotonic clock here, so
;  "order" is simply a function of elapsed time.
; ===========================================================================

_frame:
%if OFFSCREEN == 0
    call    bind_fbo                    ; render at the original size, in the
%endif                                  ; corner of the back buffer
    call    get_ms                      ; rax = milliseconds since start
    mov     r13, rax

    mov     [wTemp], eax                ; ts = ms / 20
    fild    dword [wTemp]
    fidiv   word [i20]
    fstp    dword [v_ts]

    mov     rax, r13                    ; order = ms / (114 * 16)
    xor     edx, edx
    mov     ecx, ROW_MS * ROWS
    div     rcx
    cmp     eax, ORDERS
    jae     exitIntro
    mov     [v_order], eax
    lea     rsi, [orderStuff]
    mov     al, [rsi + rax]
    mov     [v_flags], al

%if OFFSCREEN == 0
    mov     edi, kCGEventSourceStateHIDSystemState
    mov     esi, KEY_ESC
    CALLI   CGEventSourceKeyState
    test    al, al
    jnz     exitIntro
%endif

; ---- frame set-up ---------------------------------------------------------
    movss   xmm0, [f0_6]
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glClearColor
    mov     edi, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
    CALLI   glClear
    mov     edi, GL_BLEND
    CALLI   glEnable
    mov     edi, GL_LIGHT0
    CALLI   glEnable
    mov     edi, GL_COLOR_MATERIAL
    CALLI   glEnable
    mov     edi, GL_LINE_SMOOTH
    CALLI   glEnable
    mov     edi, GL_POINT_SMOOTH
    CALLI   glEnable
    mov     edi, GL_PROJECTION
    CALLI   glMatrixMode
    CALLI   glLoadIdentity
    movsd   xmm0, [d_fovy]
    movsd   xmm1, [d_aspect]
    movsd   xmm2, [d_znear]
    movsd   xmm3, [d_zfar]
    CALLI   gluPerspective
    mov     edi, GL_MODELVIEW
    CALLI   glMatrixMode
    CALLI   glLoadIdentity

; ===========================================================================
;  Effect 1 - lit rubik grid over random background lines, plus flares
; ===========================================================================
    test    byte [v_flags], 1
    jz      .efecte2

    movss   xmm0, [f5_0]
    CALLI   glLineWidth
    movss   xmm0, [f1_0]
    movss   xmm1, [f0_9]
    xorps   xmm2, xmm2
    CALLI   glColor3f
    mov     edi, GL_LINES
    CALLI   glBegin

    mov     r14d, 25
.backlines:
    fild    word [i17]
    call    frand
    fstp    dword [rsp-4]
    mov     r15d, [rsp-4]
    movss   xmm0, [fm30_0]
    movd    xmm1, r15d
    movss   xmm2, [fm20_0]
    CALLI   glVertex3f
    movss   xmm0, [f30_0]
    movd    xmm1, r15d
    movss   xmm2, [fm20_0]
    CALLI   glVertex3f
    dec     r14d
    jnz     .backlines
    CALLI   glEnd

    mov     edi, GL_DEPTH_TEST
    CALLI   glEnable
    mov     edi, GL_LIGHTING
    CALLI   glEnable
    movss   xmm0, [f8_0]
    CALLI   glLineWidth
    movss   xmm0, [f1_0]
    xorps   xmm1, xmm1
    movss   xmm2, [f0_2]
    CALLI   glColor3f
    mov     edi, GL_BLEND
    CALLI   glDisable

    mov     eax, 1                      ; scale 1x1, no offset, solid pass
    mov     r10d, 1
    xor     r11d, r11d
    xor     r9d, r9d
    call    drawRubic

    mov     edi, GL_LIGHTING
    CALLI   glDisable
    mov     edi, GL_DEPTH_TEST
    CALLI   glDisable
    mov     edi, GL_TEXTURE_2D
    CALLI   glEnable
    mov     edi, GL_BLEND
    CALLI   glEnable
    mov     edi, GL_ONE
    mov     esi, GL_ONE
    CALLI   glBlendFunc
    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    CALLI   glColor3f

    lea     r13, [px]
    mov     r14d, MAX_PARTICLES
.part_ef1:
    mov     r15d, [r13]
    CALLI   glLoadIdentity
    movd    xmm0, r15d
    movss   xmm1, [f3_0]
    movss   xmm2, [fm4_2]
    CALLI   glTranslatef
    mov     edi, 1
    CALLI   glCallList
    CALLI   glLoadIdentity
    mov     eax, r15d                   ; mirrored copy
    xor     eax, 0x80000000
    movd    xmm0, eax
    movss   xmm1, [fm3_0]
    movss   xmm2, [fm4_2]
    CALLI   glTranslatef
    mov     edi, 1
    CALLI   glCallList
    add     r13, 4
    dec     r14d
    jnz     .part_ef1

    mov     edi, GL_TEXTURE_2D
    CALLI   glDisable
    mov     edi, GL_SRC_ALPHA
    mov     esi, GL_ONE_MINUS_SRC_ALPHA
    CALLI   glBlendFunc
    movss   xmm0, [f8_0]
    CALLI   glLineWidth
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    CALLI   glColor3f
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    movss   xmm2, [f10_0]
    CALLI   glTranslatef

; ===========================================================================
;  Effect 2 - lit sine sheet plus three line strips
; ===========================================================================
.efecte2:
    test    byte [v_flags], 2
    jz      .efecte3

    fld     dword [v_ts]
    fidiv   word [i18]
    fstp    dword [v_lts]

    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    movss   xmm2, [fm3_2]
    CALLI   glTranslatef

    fld     dword [v_lts]
    fimul   word [i10]
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef

    mov     edi, GL_LIGHTING
    CALLI   glEnable
    movss   xmm0, [f0_8]
    movss   xmm1, [f0_4]
    movss   xmm2, [f0_2]
    CALLI   glColor3f
    mov     edi, GL_QUADS
    CALLI   glBegin

    mov     word [v_i], -15
    mov     r14d, 30
.y_ef2:
    fild    word [v_i]
    fmul    dword [f0_3]
    fst     dword [v_y]
    fld     st0
    fld     dword [v_lts]
    fld     st0
    faddp   st2, st0
    fadd    dword [f0_3]
    faddp   st2, st0
    fcos
    fstp    dword [v_t1]
    fsin
    fstp    dword [v_t2]

    mov     word [v_j], -15
    mov     r15d, 30
.x_ef2:
    fild    word [v_j]
    fmul    dword [f0_3]
    fst     dword [v_x]
    fld     st0
    fld     dword [v_lts]
    fld     st0
    faddp   st2, st0
    fadd    dword [f0_3]
    faddp   st2, st0
    fsin
    fstp    dword [v_k1]
    fcos
    fstp    dword [v_k2]

    fld     dword [v_k1]                ; normal and first corner share a value
    fadd    dword [v_t1]
    FPOP    2
    movss   xmm0, [v_x]
    movss   xmm1, [v_y]
    CALLI   glNormal3f

    fld     dword [v_k1]
    fadd    dword [v_t1]
    FPOP    2
    movss   xmm0, [v_x]
    movss   xmm1, [v_y]
    CALLI   glVertex3f

    fld     dword [v_k2]
    fadd    dword [v_t1]
    FPOP    2
    fld     dword [v_x]
    fadd    dword [f0_3]
    FPOP    0
    movss   xmm1, [v_y]
    CALLI   glVertex3f

    fld     dword [v_k2]
    fadd    dword [v_t2]
    FPOP    2
    fld     dword [v_y]
    fadd    dword [f0_3]
    FPOP    1
    fld     dword [v_x]
    fadd    dword [f0_3]
    FPOP    0
    CALLI   glVertex3f

    fld     dword [v_k1]
    fadd    dword [v_t2]
    FPOP    2
    fld     dword [v_y]
    fadd    dword [f0_3]
    FPOP    1
    movss   xmm0, [v_x]
    CALLI   glVertex3f

    inc     word [v_j]
    dec     r15d
    jnz     .x_ef2
    inc     word [v_i]
    dec     r14d
    jnz     .y_ef2
    CALLI   glEnd

    mov     edi, GL_DEPTH_TEST
    CALLI   glDisable
    mov     edi, GL_LIGHTING
    CALLI   glDisable
    movss   xmm0, [f8_0]
    CALLI   glLineWidth
    movss   xmm0, [f1_0]
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    CALLI   glColor3f
    CALLI   glLoadIdentity
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    movss   xmm2, [fm3_2]
    CALLI   glTranslatef

    mov     word [v_i], 0
    mov     r14d, 3
.lines_ef2:
    mov     edi, GL_LINE_STRIP
    CALLI   glBegin
    mov     word [v_j], -15
    mov     r15d, 30
.lef2_1:
    fild    word [v_j]
    fmul    dword [f0_3]
    fst     dword [v_x]
    fadd    dword [v_lts]
    fild    word [v_i]
    fimul   word [i30]
    faddp   st1, st0
    fsin
    fst     dword [rsp-12]              ; z
    fld     dword [v_x]
    fadd    st0, st0
    fadd    dword [v_t1]
    fstp    dword [rsp-8]               ; y
    fiadd   word [v_i]
    fstp    dword [rsp-4]               ; x
    movss   xmm0, [rsp-4]
    movss   xmm1, [rsp-8]
    movss   xmm2, [rsp-12]
    CALLI   glVertex3f
    inc     word [v_j]
    dec     r15d
    jnz     .lef2_1
    inc     word [v_i]
    dec     r14d
    jnz     .lines_ef2
    CALLI   glEnd

; ===========================================================================
;  Effect 3 - the tall spinning cube with flares either side
; ===========================================================================
.efecte3:
    test    byte [v_flags], 4
    jz      .efecte4

    mov     edi, GL_LIGHTING
    CALLI   glEnable
    mov     edi, GL_DEPTH_TEST
    CALLI   glEnable
    mov     edi, GL_DST_ALPHA
    mov     esi, GL_ONE_MINUS_SRC_ALPHA
    CALLI   glBlendFunc
    movss   xmm0, [f1_0]
    xorps   xmm1, xmm1
    movss   xmm2, [f0_2]
    CALLI   glColor3f
    CALLI   glLoadIdentity

    fld     dword [v_ts]
    fidiv   word [i64]
    fsin
    fimul   word [i20]
    fst     dword [v_x]                 ; reused by the flares below
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef

    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    movss   xmm2, [fm5_0]
    CALLI   glTranslatef
    movss   xmm0, [f1_0]
    movss   xmm1, [f8_0]
    movss   xmm2, [f1_0]
    CALLI   glScalef
    movss   xmm0, [v_ts]
    movss   xmm1, [f1_0]
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    call    drawCube

    mov     edi, GL_LIGHTING
    CALLI   glDisable
    mov     edi, GL_FRONT_AND_BACK
    mov     esi, GL_LINE
    CALLI   glPolygonMode
    movss   xmm0, [f8_0]
    CALLI   glLineWidth
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    CALLI   glColor3f
    call    drawCube
    mov     edi, GL_FRONT_AND_BACK
    mov     esi, GL_FILL
    CALLI   glPolygonMode

    mov     edi, GL_DEPTH_TEST
    CALLI   glDisable
    mov     edi, GL_TEXTURE_2D
    CALLI   glEnable
    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    CALLI   glColor3f
    mov     edi, GL_ONE
    mov     esi, GL_ONE
    CALLI   glBlendFunc

    lea     r13, [px]
    mov     r14d, MAX_PARTICLES
.part_ef3:
    CALLI   glLoadIdentity
    movss   xmm0, [v_x]
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    fld     dword [r13]
    fld     dword [v_x]
    fidiv   word [i10]
    fsubp   st1, st0
    FPOP    1
    movss   xmm0, [fm3_0]
    movss   xmm2, [fm5_0]
    CALLI   glTranslatef
    mov     edi, 1
    CALLI   glCallList
    movss   xmm0, [f6_0]
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    CALLI   glTranslatef
    mov     edi, 1
    CALLI   glCallList
    add     r13, 4
    dec     r14d
    jnz     .part_ef3

    mov     edi, GL_TEXTURE_2D
    CALLI   glDisable

; ===========================================================================
;  Effect 4 - wide rubik grid with orbiting flares and streaks
; ===========================================================================
.efecte4:
    test    byte [v_flags], 8
    jz      .efecte5

    mov     edi, GL_DST_ALPHA
    mov     esi, GL_ONE_MINUS_SRC_ALPHA
    CALLI   glBlendFunc
    movss   xmm0, [f2_0]
    CALLI   glLineWidth
    movss   xmm0, [f1_0]
    movss   xmm1, [f0_4]
    xorps   xmm2, xmm2
    CALLI   glColor3f

    mov     eax, 2
    mov     r10d, 3
    mov     r11d, -7
    xor     r9d, r9d
    call    drawRubic

    mov     edi, GL_TEXTURE_2D
    CALLI   glEnable
    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    CALLI   glColor3f

    lea     r13, [px]
    mov     word [v_i], 0
    mov     r14d, MAX_PARTICLES
.part_ef4:
    CALLI   glLoadIdentity
    movss   xmm0, [f1_5]
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    CALLI   glTranslatef
    fild    word [v_i]
    fsin
    fimul   word [i180]
    fadd    dword [v_ts]
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    movss   xmm0, [fm0_8]
    xorps   xmm1, xmm1
    movss   xmm2, [r13]
    CALLI   glTranslatef
    mov     edi, 1
    CALLI   glCallList
    movss   xmm0, [f1_6]
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    CALLI   glTranslatef
    mov     edi, 1
    CALLI   glCallList
    add     r13, 4
    inc     word [v_i]
    dec     r14d
    jnz     .part_ef4

    mov     edi, GL_TEXTURE_2D
    CALLI   glDisable
    movss   xmm0, [f1_0]
    CALLI   glLineWidth
    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    movss   xmm3, [f0_2]
    CALLI   glColor4f
    mov     edi, GL_SRC_ALPHA
    mov     esi, GL_ONE_MINUS_SRC_ALPHA
    CALLI   glBlendFunc

    lea     r13, [px]
    mov     word [v_i], 0
    mov     r14d, MAX_PARTICLES
.part_ef4_lin:
    CALLI   glLoadIdentity
    movss   xmm0, [f1_5]
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    CALLI   glTranslatef
    fild    word [v_i]
    fsin
    fimul   word [i180]
    fadd    dword [v_ts]
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    mov     edi, GL_LINES
    CALLI   glBegin
    movss   xmm0, [fm0_8]
    xorps   xmm1, xmm1
    movss   xmm2, [r13]
    CALLI   glVertex3f
    movss   xmm0, [f0_8]
    xorps   xmm1, xmm1
    movss   xmm2, [r13]
    CALLI   glVertex3f
    CALLI   glEnd
    add     r13, 4
    inc     word [v_i]
    dec     r14d
    jnz     .part_ef4_lin

; ===========================================================================
;  Effect 5 - dense wireframe grid, flare swarm and the smeared glyph
; ===========================================================================
.efecte5:
    test    byte [v_flags], 16
    jz      .efecte6

    fld     dword [v_ts]
    fidiv   word [i5]
    fstp    dword [v_lts]

    movss   xmm0, [f1_0]
    CALLI   glLineWidth

    mov     eax, 9
    mov     r10d, 9
    xor     r11d, r11d
    mov     r9d, 1                      ; outline pass only
    call    drawRubic

    mov     edi, GL_ONE
    mov     esi, GL_ONE
    CALLI   glBlendFunc
    mov     edi, GL_TEXTURE_2D
    CALLI   glEnable
    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    CALLI   glColor3f

    lea     r13, [px]
    mov     word [v_i], 0
    mov     r14d, MAX_PARTICLES
.part_ef5:
    CALLI   glLoadIdentity
    fild    word [v_i]
    fsin
    fimul   word [i180]
    fimul   word [i17]
    fadd    dword [v_lts]
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    movss   xmm0, [r13]
    xorps   xmm1, xmm1
    movss   xmm2, [r13]
    CALLI   glTranslatef
    mov     edi, 1
    CALLI   glCallList
    add     r13, 4
    inc     word [v_i]
    dec     r14d
    jnz     .part_ef5

    mov     edi, GL_TEXTURE_2D
    CALLI   glDisable
    mov     edi, GL_SRC_ALPHA
    mov     esi, GL_ONE_MINUS_SRC_ALPHA
    CALLI   glBlendFunc
    movss   xmm0, [f8_0]
    CALLI   glLineWidth
    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    movss   xmm3, [f0_3]
    CALLI   glColor4f

    mov     word [v_i], 0
    mov     r14d, 12
.drawChin_blur:
    CALLI   glLoadIdentity
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    movss   xmm2, [fm5_0]
    CALLI   glTranslatef
    fld     dword [v_lts]
    fimul   word [i4]
    fchs
    fsin
    fimul   word [i180]
    fiadd   word [v_i]
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    movss   xmm0, [f3_0]
    movss   xmm1, [f5_0]
    movss   xmm2, [f3_0]
    CALLI   glScalef
    mov     edi, 1
    call    drawChin
    inc     word [v_i]
    dec     r14d
    jnz     .drawChin_blur

; ===========================================================================
;  Effect 6 - the two glyphs swept around the screen
; ===========================================================================
.efecte6:
    test    byte [v_flags], 32
    jz      .efecte7

    fld     dword [v_ts]
    fsincos
    fimul   word [i5]
    fstp    dword [v_k1]
    fimul   word [i5]
    fstp    dword [v_k2]

    mov     edi, GL_SRC_ALPHA
    mov     esi, GL_ONE_MINUS_SRC_ALPHA
    CALLI   glBlendFunc
    movss   xmm0, [f8_0]
    CALLI   glLineWidth
    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    movss   xmm3, [f0_05]
    CALLI   glColor4f

    mov     eax, [fm4_0]
    mov     [v_t1], eax
    lea     r13, [v_k1]
    mov     r12d, 2                      ; two glyphs
.two:
    mov     word [v_i], 0
    mov     r14d, 50
.fin1:
    CALLI   glLoadIdentity
    fild    word [v_i]
    fidiv   word [i8]
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    movss   xmm0, [v_t1]
    movss   xmm1, [r13]
    movss   xmm2, [fm10_0]
    CALLI   glTranslatef
    movss   xmm0, [f4_0]
    movss   xmm1, [f6_0]
    movss   xmm2, [f3_0]
    CALLI   glScalef
    lea     edi, [r12-1]
    call    drawChin
    inc     word [v_i]
    dec     r14d
    jnz     .fin1
    add     r13, 4                      ; k1 -> k2
    mov     eax, [f4_0]
    mov     [v_t1], eax
    dec     r12d
    jnz     .two

; ===========================================================================
;  Effect 7 - flares on a bobbing ring
; ===========================================================================
.efecte7:
    test    byte [v_flags], 64
    jz      .efecte8

    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    CALLI   glColor3f
    mov     edi, GL_ONE
    mov     esi, GL_ONE
    CALLI   glBlendFunc
    mov     edi, GL_TEXTURE_2D
    CALLI   glEnable

    lea     r13, [px]
    mov     word [v_i], 0
    mov     r14d, MAX_PARTICLES
.part_ef7:
    CALLI   glLoadIdentity
    fld     dword [r13]
    fidiv   word [i10]
    fsin
    fimul   word [i180]
    fadd    dword [v_ts]
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    fld     dword [v_ts]
    fiadd   word [v_i]
    fidiv   word [i18]
    fsin
    fimul   word [i8]
    fisub   word [i10]
    FPOP    2
    movss   xmm0, [f4_0]
    xorps   xmm1, xmm1
    CALLI   glTranslatef
    mov     edi, 1
    CALLI   glCallList
    inc     word [v_i]
    add     r13, 4
    dec     r14d
    jnz     .part_ef7

    mov     edi, GL_TEXTURE_2D
    CALLI   glDisable

; ===========================================================================
;  Effect 8 - the group logo, drawn as fat smeared points
; ===========================================================================
.efecte8:
    test    byte [v_flags], 128
    jz      .endframe

    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f0_5]
    movss   xmm3, [f0_1]
    CALLI   glColor4f
    mov     edi, GL_SRC_ALPHA
    mov     esi, GL_ONE_MINUS_SRC_ALPHA
    CALLI   glBlendFunc
    movss   xmm0, [f37_0]
    CALLI   glPointSize

    mov     word [v_i], 0
    mov     r14d, 50
.fzn1:
    CALLI   glLoadIdentity
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    movss   xmm2, [fm15_0]
    CALLI   glTranslatef
    fild    word [v_i]
    fidiv   word [i8]
    FPOP    0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glRotatef
    movss   xmm0, [f4_0]
    movss   xmm1, [f6_0]
    movss   xmm2, [f3_0]
    CALLI   glScalef
    mov     edi, 8
    call    drawChin
    mov     edi, 9
    call    drawChin
    inc     word [v_i]
    dec     r14d
    jnz     .fzn1

; ---- advance the shared particle field ------------------------------------
.endframe:
    lea     rdi, [px]
    mov     ecx, MAX_PARTICLES
.particle_add:
    fild    word [i20]
    fld     dword [rdi]
    fadd    dword [rdi + MAX_PARTICLES*4]
    fcomi   st0, st1                    ; wrap once it passes 20
    jc      .no_change
    fxch    st1
    fchs
.no_change:
    fstp    dword [rdi]
    fstp    st0
    add     rdi, 4
    dec     ecx
    jnz     .particle_add

%if OFFSCREEN
    call    grab_frame
%elif WINDOWED
    call    blit_frame
    call    window_flush
%else
    call    blit_frame
    mov     rdi, [ctx]
    CALLI   CGLFlushDrawable
%endif
    jmp     _frame
