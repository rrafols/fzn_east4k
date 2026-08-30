; ---------------------------------------------------------------------------
;  Replacement for the Win32 IMPORTS.INC.
;
;  Order is deliberate: the first 32 entries are the ones called from inner
;  loops, and RBX points at imports+128 so those reach through a 3-byte
;  "call [rbx+disp8]" instead of the 6-byte disp32 form.
; ---------------------------------------------------------------------------

%macro IMPORT_LIST 0
    ; --- hot path (kept inside the signed-byte displacement window) --------
    IMP glVertex3f
    IMP glTranslatef
    IMP glLoadIdentity
    IMP glCallList
    IMP glRotatef
    IMP glScalef
    IMP glColor3f
    IMP glBegin
    IMP glEnd
    IMP glEnable
    IMP glDisable
    IMP glNormal3f
    IMP glBlendFunc
    IMP glLineWidth
    IMP glPolygonMode
    IMP glColor4f
    IMP glMatrixMode
    IMP glClear
    IMP glClearColor
    IMP glPointSize
    IMP glTexCoord2f
    IMP glNewList
    IMP glEndList
    IMP glTexImage2D
    IMP glTexParameteri
    IMP gluPerspective
    IMP CGLFlushDrawable
    IMP CGEventSourceKeyState
    IMP clock_gettime_nsec_np
    IMP CGMainDisplayID
    IMP CGDisplayCapture
    IMP CGDisplayIDToOpenGLDisplayMask
    ; --- start-up / shutdown only ------------------------------------------
    IMP CGDisplayRelease
    IMP CGLChoosePixelFormat
    IMP CGLCreateContext
    IMP CGLSetCurrentContext
    IMP CGLSetFullScreenOnDisplay
    IMP CGLSetParameter
    IMP AudioQueueNewOutput
    IMP AudioQueueAllocateBuffer
    IMP AudioQueueEnqueueBuffer
    IMP AudioQueueStart
    IMP exit
    IMP glViewport
    IMP glCopyTexImage2D
    IMP glGenTextures
    IMP glBindTexture
    IMP glGetIntegerv
%if WINDOWED
    ; --- only linked into the windowed build --------------------------------
    IMP objc_getClass
    IMP sel_registerName
    IMP objc_msgSend
    IMP NSDefaultRunLoopMode
%endif
%if OFFSCREEN
    ; --- only linked into the offscreen frame-dump build -------------------
    IMP glGenFramebuffersEXT
    IMP glBindFramebufferEXT
    IMP glGenRenderbuffersEXT
    IMP glBindRenderbufferEXT
    IMP glRenderbufferStorageEXT
    IMP glFramebufferRenderbufferEXT
    IMP glReadPixels
    IMP fopen
    IMP fwrite
    IMP fclose
%endif
%endmacro
