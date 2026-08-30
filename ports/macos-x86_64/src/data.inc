; ---------------------------------------------------------------------------
;  Constant pool and hot variables.
;
;  The Win32 original could hand a float straight to the API as a
;  "push dword <bits>" immediate.  SysV wants it in an XMM register, which
;  would mean a rip-relative load with a four-byte displacement at every one
;  of ~400 sites - long, and (because every displacement differs) close to
;  incompressible.
;
;  So everything the effects touch constantly lives in one 256-byte block with
;  RBP parked in the middle of it (RBP needs
;  neither a REX prefix nor a SIB byte, so each access is three bytes shorter
;  than the rip-relative form).  Each name below is a macro expanding to an
;  RBP displacement, which keeps the call sites written the obvious way while
;  assembling to a one-byte offset.
; ---------------------------------------------------------------------------

%define K(x) rbp + (x - kbase - 128)

section S_DATA
align 16
kbase:
; -- single precision literals used as OpenGL arguments ---------------------
K_f0_005:   dd 0.005
K_f0_05:    dd 0.05
K_f0_1:     dd 0.1
K_f0_2:     dd 0.2
K_f0_25:    dd 0.25
K_f0_3:     dd 0.3
K_f0_4:     dd 0.4
K_f0_5:     dd 0.5
K_f0_6:     dd 0.6
K_f0_7:     dd 0.7
K_f0_8:     dd 0.8
K_f0_9:     dd 0.9
K_f1_0:     dd 1.0
K_f1_5:     dd 1.5
K_f1_6:     dd 1.6
K_f2_0:     dd 2.0
K_f2_2:     dd 2.2
K_f3_0:     dd 3.0
K_f4_0:     dd 4.0
K_f5_0:     dd 5.0
K_f6_0:     dd 6.0
K_f8_0:     dd 8.0
K_f10_0:    dd 10.0
K_f30_0:    dd 30.0
K_f37_0:    dd 37.0
K_fm0_8:    dd -0.8
K_fm1_0:    dd -1.0
K_fm2_0:    dd -2.0
K_fm3_0:    dd -3.0
K_fm3_2:    dd -3.2
K_fm4_0:    dd -4.0
K_fm4_2:    dd -4.2
K_fm5_0:    dd -5.0
K_fm10_0:   dd -10.0
K_fm15_0:   dd -15.0
K_fm20_0:   dd -20.0
K_fm30_0:   dd -30.0

; -- integer operands for fild / fimul / fidiv / fiadd / fisub --------------
K_i4:       dw 4
K_i5:       dw 5
K_i8:       dw 8
K_i10:      dw 10
K_i17:      dw 17
K_i18:      dw 18
K_i20:      dw 20                  ; the original's "timeDivider"
K_i30:      dw 30
K_i40:      dw 40
K_i64:      dw 64
K_i180:     dw 180
K_i256:     dw 256
K_i32000:   dw 32000

; -- per-frame working values (the original's globals) ----------------------
K_v_ts:     dd 0                   ; animation time
K_v_lts:    dd 0
K_v_k1:     dd 0
K_v_k2:     dd 0
K_v_t1:     dd 0
K_v_t2:     dd 0
K_v_x:      dd 0
K_v_y:      dd 0
K_wTemp:    dd 0                   ; x87 <-> integer scratch
K_holdrand: dd 0
K_v_order:  dd 0
K_v_i:      dw 0
K_v_j:      dw 0
K_v_k:      dw 0
K_fp0:      dd 0                   ; x87 -> XMM hand-over slots
K_fp1:      dd 0
K_fp2:      dd 0
K_fp3:      dd 0
K_v_flags:  db 0                   ; orderStuff bits for this order
K_rubflag:  db 0
kend:

%define f0_005   K(K_f0_005)
%define f0_05    K(K_f0_05)
%define f0_1     K(K_f0_1)
%define f0_2     K(K_f0_2)
%define f0_25    K(K_f0_25)
%define f0_3     K(K_f0_3)
%define f0_4     K(K_f0_4)
%define f0_5     K(K_f0_5)
%define f0_6     K(K_f0_6)
%define f0_7     K(K_f0_7)
%define f0_8     K(K_f0_8)
%define f0_9     K(K_f0_9)
%define f1_0     K(K_f1_0)
%define f1_5     K(K_f1_5)
%define f1_6     K(K_f1_6)
%define f2_0     K(K_f2_0)
%define f2_2     K(K_f2_2)
%define f3_0     K(K_f3_0)
%define f4_0     K(K_f4_0)
%define f5_0     K(K_f5_0)
%define f6_0     K(K_f6_0)
%define f8_0     K(K_f8_0)
%define f10_0    K(K_f10_0)
%define f30_0    K(K_f30_0)
%define f37_0    K(K_f37_0)
%define fm0_8    K(K_fm0_8)
%define fm1_0    K(K_fm1_0)
%define fm2_0    K(K_fm2_0)
%define fm3_0    K(K_fm3_0)
%define fm3_2    K(K_fm3_2)
%define fm4_0    K(K_fm4_0)
%define fm4_2    K(K_fm4_2)
%define fm5_0    K(K_fm5_0)
%define fm10_0   K(K_fm10_0)
%define fm15_0   K(K_fm15_0)
%define fm20_0   K(K_fm20_0)
%define fm30_0   K(K_fm30_0)
%define i4       K(K_i4)
%define i5       K(K_i5)
%define i8       K(K_i8)
%define i10      K(K_i10)
%define i17      K(K_i17)
%define i18      K(K_i18)
%define i20      K(K_i20)
%define i30      K(K_i30)
%define i40      K(K_i40)
%define i64      K(K_i64)
%define i180     K(K_i180)
%define i256     K(K_i256)
%define i32000   K(K_i32000)
%define v_ts     K(K_v_ts)
%define v_lts    K(K_v_lts)
%define v_k1     K(K_v_k1)
%define v_k2     K(K_v_k2)
%define v_t1     K(K_v_t1)
%define v_t2     K(K_v_t2)
%define v_x      K(K_v_x)
%define v_y      K(K_v_y)
%define wTemp    K(K_wTemp)
%define holdrand K(K_holdrand)
%define v_order  K(K_v_order)
%define v_i      K(K_v_i)
%define v_j      K(K_v_j)
%define v_k      K(K_v_k)
%define fp0      K(K_fp0)
%define fp1      K(K_fp1)
%define fp2      K(K_fp2)
%define fp3      K(K_fp3)
%define v_flags  K(K_v_flags)
%define rubflag  K(K_rubflag)

; -- referenced only a handful of times, so plain rip-relative is fine ------
section S_RODATA
align 8
f480:       dd 480.0                ; the original's screen height
c33152:     dd 33152.0
cDiv:       dd 127.71542846
c105:       dd 1.05946309436
one_i:      dd 1
cubekey:    dd 0.0, 1.0, -1.0
volgain:    dd 0.0, 0.398107, 0.446684, 1.0   ; -10000, -800, -700, 0 (1/100 dB)
mastergain: dd 64.0
rate_f:     dd 44100.0
d_fovy:     dq 80.0
d_znear:    dq 0.1
d_zfar:     dq 100.0

; -- CGL pixel format request -----------------------------------------------
section S_DATA
align 8
pfattr:     dd kCGLPFADoubleBuffer
            dd kCGLPFAFullScreen
            dd kCGLPFAAccelerated
            dd kCGLPFADisplayMask
dmask:      dd 0                    ; patched with the real display mask
            dd kCGLPFAColorSize, 24
            dd kCGLPFADepthSize, 32
            dd 0
d_aspect:   dq 1.3333333333333333

; -- AudioStreamBasicDescription: 44100 Hz, mono, signed 16-bit packed ------
asbd:       dq 0                    ; mSampleRate, filled at start-up
            dd kAudioFormatLinearPCM
            dd kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked
            dd 2                    ; mBytesPerPacket
            dd 1                    ; mFramesPerPacket
            dd 2                    ; mBytesPerFrame
            dd 1                    ; mChannelsPerFrame
            dd 16                   ; mBitsPerChannel
            dd 0                    ; mReserved
d_rate:     dq 44100.0

section .bss
alignb 16
flare:      resd FLARE_DIM * FLARE_DIM * 4      ; RGBA float lens-flare texture
px:         resd MAX_PARTICLES * 2              ; particle position then velocity
sampbuf:    resb NSAMPLES * 20000               ; the eight generated instruments
freqtable:  resd 96
voices:     resd NCHANNELS * 4                  ; index, position, step, gain
cubef:      resd 90                             ; unpacked cube vertices

ctx:        resq 1                              ; CGLContextObj
pixfmt:     resq 1
npix:       resq 1
aq:         resq 1                              ; AudioQueueRef
aqbuf:      resq 1                              ; AudioQueueBufferRef
t_start:    resq 1                              ; clock at start, nanoseconds
dispid:     resd 1
scenetex:   resd 1                              ; the 640x480 frame, as a texture
vpbuf:      resd 4                              ; the drawable GL handed us
blitx:      resd 1                              ; where it lands on the display
blity:      resd 1
blitw:      resd 1
blith:      resd 1
