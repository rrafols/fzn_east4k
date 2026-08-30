; ---------------------------------------------------------------------------
;  Constants used by the intro: the OpenGL enums the original pulled from
;  OPENGL.INC, plus the CGL / CoreGraphics / AudioToolbox values that replace
;  the Win32 + DirectSound ones.
; ---------------------------------------------------------------------------

; -- OpenGL -----------------------------------------------------------------
%define GL_DEPTH_BUFFER_BIT     0x00000100
%define GL_COLOR_BUFFER_BIT     0x00004000
%define GL_POINTS               0x0000
%define GL_LINES                0x0001
%define GL_LINE_STRIP           0x0003
%define GL_QUADS                0x0007
%define GL_ONE                  1
%define GL_SRC_ALPHA            0x0302
%define GL_ONE_MINUS_SRC_ALPHA  0x0303
%define GL_DST_ALPHA            0x0304
%define GL_FLOAT                0x1406
%define GL_FRONT_AND_BACK       0x0408
%define GL_POINT_SMOOTH         0x0B10
%define GL_LINE_SMOOTH          0x0B20
%define GL_LIGHTING             0x0B50
%define GL_COLOR_MATERIAL       0x0B57
%define GL_DEPTH_TEST           0x0B71
%define GL_BLEND                0x0BE2
%define GL_TEXTURE_2D           0x0DE1
%define GL_LIGHT0               0x4000
%define GL_COMPILE              0x1300
%define GL_MODELVIEW            0x1700
%define GL_PROJECTION           0x1701
%define GL_RGBA                 0x1908
%define GL_LINE                 0x1B01
%define GL_FILL                 0x1B02
%define GL_LINEAR               0x2601
%define GL_TEXTURE_MIN_FILTER   0x2801
%define GL_TEXTURE_MAG_FILTER   0x2800
%define GL_RGBA8                0x8058
%define GL_DEPTH_COMPONENT24    0x81A6
%define GL_FRAMEBUFFER_EXT      0x8D40
%define GL_READ_FRAMEBUFFER     0x8CA8
%define GL_DRAW_FRAMEBUFFER     0x8CA9
%define GL_RENDERBUFFER_EXT     0x8D41
%define GL_COLOR_ATTACHMENT0    0x8CE0
%define GL_DEPTH_ATTACHMENT     0x8D00
%define GL_VIEWPORT             0x0BA2
%define GL_RGB                  0x1907
%define GL_UNSIGNED_BYTE        0x1401

; -- CGL pixel format attributes --------------------------------------------
%define kCGLPFADoubleBuffer     5
%define kCGLPFAColorSize        8
%define kCGLPFADepthSize        12
%define kCGLPFAFullScreen       54
%define kCGLPFAAccelerated      73
%define kCGLPFADisplayMask      84
%define kCGLCPSwapInterval      222

; -- CoreGraphics -----------------------------------------------------------
%define kCGEventSourceStateHIDSystemState  1
%define KEY_ESC                 53

; -- AudioToolbox -----------------------------------------------------------
%define kAudioFormatLinearPCM   0x6C70636D          ; 'lpcm'
%define kLinearPCMFormatFlagIsSignedInteger 4
%define kLinearPCMFormatFlagIsPacked        8

; -- Intro parameters (unchanged from the Win32 original) -------------------
%define NSAMPLES        8
%define NCHANNELS       8
%define MAX_PARTICLES   100
%define ORDERS          63
%define ROWS            16
%define ROW_MS          114
%define SND_RATE        44100
%define ROW_SAMPLES     (SND_RATE * ROW_MS / 1000)
%define TOTAL_SAMPLES   (ORDERS * ROWS * ROW_SAMPLES)
%define FLARE_DIM       128
%define RENDER_W        640             ; the resolution the original forced
%define RENDER_H        480
