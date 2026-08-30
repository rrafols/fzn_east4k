; ---------------------------------------------------------------------------
;  Geometry helpers and the small maths routines from the original.
; ---------------------------------------------------------------------------

; 2^x via the x87 partial-remainder trick, exactly as the original fexp.
; in : st0 = x    out: st0 = e**x
fexp:
    fldl2e
    fmulp   st1, st0
    fld1
    fld     st1
    fprem
    f2xm1
    faddp   st1, st0
    fscale
    fxch    st1
    fstp    st0
    ret

; The Microsoft C rand(), which is what the original used to seed the flare
; particles.  out: eax = 0..0x7fff
rand:
    mov     eax, [holdrand]
    imul    eax, eax, 214013
    add     eax, 2531011
    mov     [holdrand], eax
    and     eax, 0x7FFF
    ret

; in : st0 = max     out: st0 = max * (rand()/32000 - 0.5)
frand:
    call    rand
    mov     [wTemp], eax
    fild    dword [wTemp]
    fidiv   word [i32000]
    fsub    dword [f0_5]
    fmulp   st1, st0
    ret

; Expand the 2-bit packed cube into 90 floats.  Called once at start-up.
unpackcube:
    lea     rsi, [cubepack]
    lea     rdi, [cubef]
    lea     r8, [cubekey]
    xor     ecx, ecx                    ; component index
.next:
    mov     eax, ecx
    shr     eax, 2
    movzx   edx, byte [rsi + rax]
    mov     eax, ecx
    and     eax, 3
    add     eax, eax
    xchg    ecx, eax
    shr     edx, cl                     ; select the component's two bits
    xchg    ecx, eax
    and     edx, 3
    mov     eax, [r8 + rdx*4]
    stosd
    inc     ecx
    cmp     ecx, 90
    jne     .next
    ret

; The unit cube: six quads, each preceded by its face normal.
drawCube:
    push    r12
    push    r14
    push    r15
    mov     edi, GL_QUADS
    CALLI   glBegin
    lea     r12, [cubef]
    mov     r15d, 6
.face:
    movss   xmm0, [r12]
    movss   xmm1, [r12+4]
    movss   xmm2, [r12+8]
    add     r12, 12
    CALLI   glNormal3f
    mov     r14d, 4
.vert:
    movss   xmm0, [r12]
    movss   xmm1, [r12+4]
    movss   xmm2, [r12+8]
    add     r12, 12
    CALLI   glVertex3f
    dec     r14d
    jnz     .vert
    dec     r15d
    jnz     .face
    CALLI   glEnd
    pop     r15
    pop     r14
    pop     r12
    ret

; 3x3x3 grid of cubes, drawn solid and then again as black wireframe.
; in : eax = x scale, r10d = y scale, r11d = x offset, r9b = 0 to draw the
;      solid pass (non-zero draws the outline pass only)
drawRubic:
    push    r12
    push    r13
    push    r14
    push    r15
    push    rax
    mov     [rubflag], r9b

    mov     [wTemp], eax
    fild    dword [wTemp]
    fstp    dword [v_k1]
    mov     [wTemp], r10d
    fild    dword [wTemp]
    fstp    dword [v_k2]
    mov     [wTemp], r11d
    fild    dword [wTemp]
    fisub   word [i5]
    fstp    dword [v_t1]

    xor     r12d, r12d                    ; solid pass, then outline pass
.outer:
    cmp     byte [rubflag], 0
    jne     .nextLoop

    mov     word [v_k], -1
    mov     r13d, 3
.z:
    mov     word [v_j], -1
    mov     r14d, 3
.y:
    mov     word [v_i], -1
    mov     r15d, 3
.x:
    CALLI   glLoadIdentity

    movss   xmm0, [v_t1]
    xorps   xmm1, xmm1
    movss   xmm2, [fm10_0]
    CALLI   glTranslatef

    movss   xmm0, [v_k1]
    movss   xmm1, [v_k2]
    movss   xmm2, [f1_0]
    CALLI   glScalef

    movss   xmm0, [v_ts]
    xorps   xmm1, xmm1
    movss   xmm2, [f1_0]
    movss   xmm3, [f1_0]
    CALLI   glRotatef

    fild    word [v_i]
    fmul    dword [f2_2]
    FPOP    0
    fild    word [v_j]
    fmul    dword [f2_2]
    FPOP    1
    fild    word [v_k]
    fmul    dword [f2_2]
    FPOP    2
    CALLI   glTranslatef

    call    drawCube

    inc     word [v_i]
    dec     r15d
    jnz     .x
    inc     word [v_j]
    dec     r14d
    jnz     .y
    inc     word [v_k]
    dec     r13d
    jnz     .z

.nextLoop:
    mov     edi, GL_FRONT_AND_BACK
    mov     esi, GL_LINE
    CALLI   glPolygonMode
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    CALLI   glColor3f

    mov     byte [rubflag], 0
    inc     r12d
    cmp     r12d, 2
    jne     .outer

    mov     edi, GL_FRONT_AND_BACK
    mov     esi, GL_FILL
    CALLI   glPolygonMode

    pop     rax
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    ret

; Draws one of the stroke glyphs.  in: edi = glyph index
drawChin:
    push    r12
    push    r14
    push    r15

    lea     r15, [chin1]
    mov     r14d, 10*2
    test    edi, edi
    jnz     .okChar
    lea     r15, [chin2]
    mov     r14d, 7*2
    jmp     .noFuzz
.okChar:
    cmp     edi, 8
    jl      .noFuzz
    lea     r15, [fznchar]
    mov     r14d, 3*2
.noFuzz:
    mov     r12d, GL_LINES
    cmp     edi, 9
    jne     .doStart
    mov     r12d, GL_POINTS
.doStart:
    mov     edi, r12d
    CALLI   glBegin
.loop:
    movzx   eax, byte [r15]             ; first byte  -> x
    movzx   edx, byte [r15+1]           ; second byte -> y
    add     r15, 2
    sub     eax, 128
    sub     edx, 128
    movsx   eax, al
    movsx   edx, dl
    add     eax, eax
    add     edx, edx

    mov     [wTemp], eax
    fild    dword [wTemp]
    fidiv   word [i256]
    FPOP    0                           ; x
    mov     [wTemp], edx
    fild    dword [wTemp]
    fchs
    fidiv   word [i256]
    FPOP    1                           ; y
    xorps   xmm2, xmm2
    CALLI   glVertex3f

    dec     r14d
    jnz     .loop
    CALLI   glEnd

    pop     r15
    pop     r14
    pop     r12
    ret



; The intro renders into the bottom-left 640x480 of the back buffer - exactly
; the resolution it was written for, so line widths and point sizes are the ones
; it chose - and that corner is then copied into a texture and drawn across the
; display.  glCopyTexImage2D avoids needing a framebuffer object at all, which
; is seven imports and a lot of set-up saved.
make_tex:
    push    r12
    mov     edi, 1
    lea     rsi, [scenetex]
    CALLI   glGenTextures
    mov     edi, GL_TEXTURE_2D
    mov     esi, [scenetex]
    CALLI   glBindTexture
    mov     edi, GL_TEXTURE_2D
    mov     esi, GL_TEXTURE_MIN_FILTER
    mov     edx, GL_LINEAR
    CALLI   glTexParameteri
    mov     edi, GL_TEXTURE_2D
    mov     esi, GL_TEXTURE_MAG_FILTER
    mov     edx, GL_LINEAR
    CALLI   glTexParameteri
    mov     edi, GL_TEXTURE_2D          ; the flare lives in texture object 0
    xor     esi, esi
    CALLI   glBindTexture
    pop     r12
    ret

; Render at the original size, in the corner of the back buffer.
bind_fbo:
    push    r12
    xor     edi, edi
    xor     esi, esi
    mov     edx, RENDER_W
    mov     ecx, RENDER_H
    CALLI   glViewport
    pop     r12
    ret

; Copy that corner into the texture, wipe the screen, and draw it scaled.
blit_frame:
    push    r12
    mov     edi, GL_TEXTURE_2D
    mov     esi, [scenetex]
    CALLI   glBindTexture

    ; glCopyTexImage2D(target, level, internalformat, x, y, w, h, border)
    sub     rsp, 16
    mov     qword [rsp], RENDER_H
    mov     qword [rsp+8], 0
    mov     edi, GL_TEXTURE_2D
    xor     esi, esi
    mov     edx, GL_RGBA
    xor     ecx, ecx
    xor     r8d, r8d
    mov     r9d, RENDER_W
    CALLI   glCopyTexImage2D
    add     rsp, 16

    xor     edi, edi                    ; wipe the whole drawable, so the
    xor     esi, esi                    ; corner we just rendered into does
    mov     edx, [vpbuf+8]              ; not show through the letterbox bars
    mov     ecx, [vpbuf+12]
    CALLI   glViewport
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    movss   xmm3, [f1_0]
    CALLI   glClearColor
    mov     edi, GL_COLOR_BUFFER_BIT
    CALLI   glClear

    mov     edi, [blitx]
    mov     esi, [blity]
    mov     edx, [blitw]
    mov     ecx, [blith]
    CALLI   glViewport

    mov     edi, GL_DEPTH_TEST
    CALLI   glDisable
    mov     edi, GL_LIGHTING
    CALLI   glDisable
    mov     edi, GL_BLEND
    CALLI   glDisable
    mov     edi, GL_TEXTURE_2D
    CALLI   glEnable

    mov     edi, GL_PROJECTION
    CALLI   glMatrixMode
    CALLI   glLoadIdentity
    mov     edi, GL_MODELVIEW
    CALLI   glMatrixMode
    CALLI   glLoadIdentity

    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    movss   xmm2, [f1_0]
    CALLI   glColor3f
    mov     edi, GL_QUADS
    CALLI   glBegin

    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    CALLI   glTexCoord2f
    movss   xmm0, [fm1_0]
    movss   xmm1, [fm1_0]
    xorps   xmm2, xmm2
    CALLI   glVertex3f

    movss   xmm0, [f1_0]
    xorps   xmm1, xmm1
    CALLI   glTexCoord2f
    movss   xmm0, [f1_0]
    movss   xmm1, [fm1_0]
    xorps   xmm2, xmm2
    CALLI   glVertex3f

    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    CALLI   glTexCoord2f
    movss   xmm0, [f1_0]
    movss   xmm1, [f1_0]
    xorps   xmm2, xmm2
    CALLI   glVertex3f

    xorps   xmm0, xmm0
    movss   xmm1, [f1_0]
    CALLI   glTexCoord2f
    movss   xmm0, [fm1_0]
    movss   xmm1, [f1_0]
    xorps   xmm2, xmm2
    CALLI   glVertex3f
    CALLI   glEnd

    mov     edi, GL_TEXTURE_2D          ; hand texture 0 back to the flare
    xor     esi, esi
    CALLI   glBindTexture
    mov     edi, GL_TEXTURE_2D
    CALLI   glDisable
    pop     r12
    ret
