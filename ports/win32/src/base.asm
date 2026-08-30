%include "macros.inc"
%include "opengl.inc"
%include "dsound.inc"
%include "winbase.inc"
%include "wingdi.inc"
%include "winuser.inc"	
%include "const.inc"
%include "pehdr.inc"

%define NSAMPLES	8
%define FULLSCREEN	1
%define	SCREEN_WIDTH	640
%define	SCREEN_HEIGHT	480


%define COMPATIBLE_MODE	1
%define	MAX_PARTICLES	100

%define WAVE_FORMAT_PCM     	1
%define maxOrderList		75

START_PROGRAM

_OpenGL32Name	db	'OPENGL32.DLL',0
_glu32Name	db	'GLU32.DLL',0
_DSoundName	db	'DSOUND.DLL',0
_gdi32Name	db	'GDI32.DLL',0

parameter	db	0

%if COMPATIBLE_MODE
Pixelfd		dw	40		;size
		dw	1		;version
		dd	PFD_DRAW_TO_WINDOW + PFD_SUPPORT_OPENGL + PFD_DOUBLEBUFFER
		db	PFD_TYPE_RGBA	;iPixelType
		db	32		;ColorBits
		db	0,0,0,0,0,0	;Color+Shift bits (ignored)
		db	0,0		;Alpha bits & Shift bits (ignored)
		db	0		;No Accumulation Buffer
		db	0,0,0,0		;Accum Bits (ignored)
		db	32		;16 bits ZBuffer
		db	0		;No Stencil Buffer
		db	0		;No Auxiliary Buffer
		db	PFD_MAIN_PLANE	;iLayer Type
		db	0		;Reserved
		dd	0,0,0		;Layer Masks Ignored
%endif

bufdesc		dd	20		;size
		dd	DSBCAPS_PRIMARYBUFFER + DSBCAPS_STICKYFOCUS	;flags
		dd	0		;BufferBytes
		dd	0		;Reserved
		dd	0		;lpwfxFormat		

waveformat	dw	WAVE_FORMAT_PCM	;wFormatTag
		dw	1		;nChannels
		dd	44100		;SamplesPerSec
		dd	44100		;nAvgBytesPerSec
		dw	1		;nBlockAlign
		dw	8		;wBitsPerSample
		dw	0		;cbSize

_EntryPoint:	
	push	ebp
	

	mov	ebp,ABS(oglFunc)
	mov	edi,ABS(dmScreen)
	
%if FULLSCREEN	
	mov	dword [edi+ 36],148			;size
	mov	dword [edi+108],SCREEN_WIDTH			;width
	mov	dword [edi+112],SCREEN_HEIGHT			;height
	mov	dword [edi+104],32			;bits
	mov	dword [edi+40],DM_BITSPERPEL + DM_PELSWIDTH + DM_PELSHEIGHT
		
	xor	eax,eax

	push	eax
	push	dword _ImageBase
	push	eax
	push	eax
	push	dword SCREEN_HEIGHT
	push	dword SCREEN_WIDTH
	push	CW_USEDEFAULT
	push	CW_USEDEFAULT
	push	dword WS_POPUP | WS_VISIBLE |WS_CLIPSIBLINGS | WS_CLIPCHILDREN
	push	ABS(editName)
	push	ABS(editName)
	push	dword WS_EX_APPWINDOW	
		
	push	byte 0	
	push	dword CDS_FULLSCREEN
	push	edi
	INVOKER ebp+oglImports.ChangeDisplaySettings
	INVOKER ebp+oglImports.ShowCursor
	INVOKER ebp+oglImports.CreateWindowEx
	
;	INVOKER	ABS(ChangeDisplaySettings)
;	INVOKER ABS(ShowCursor)	
;	INVOKER ABS(CreateWindowEx)
	xor	ebx,ebx
%else
	xor	ebx,ebx
	mov	eax,ABS(editName)
	mov	edx,CW_USEDEFAULT	
	INVOKER ABS(CreateWindowEx),dword WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,eax,eax,WS_OVERLAPPEDWINDOW | WS_VISIBLE |	WS_CLIPSIBLINGS | WS_CLIPCHILDREN,edx,edx,dword SCREEN_WIDTH,dword SCREEN_HEIGHT,ebx,ebx,dword _ImageBase,ebx	
%endif
	xchg	eax,edi

						;edi=hWnd
	mov	esi,ABS(dsound)
	push	ebx
	push	esi
	push	ebx
	INVOKER	ebp+oglImports.DirectSoundCreate
	;INVOKER ABS(DirectSoundCreate),ebx,esi,ebx
	
	mov	ebx,[esi]			;ebx -> [dsound]
	mov	esi,[ebx]			;esi -> [dsound->lpVtbl]
	push	dword DSSCL_EXCLUSIVE | DSSCL_PRIORITY
	push	edi				;hwnd
	push	ebx
	call	dword [esi+24]			;SetCooperativeLevel(dsound,hwnd,DSSCL_EXCLUSIVE | DSSCL_PRIORITY)	

	push	edi
	INVOKER	ebp+oglImports.GetDC
	;INVOKER	ABS(GetDC),edi
	xchg	eax,esi				;esi=hDC

	
;	push	esi

%if COMPATIBLE_MODE
	mov	ebx,ABS(Pixelfd)
	
	push	ebx
	push	esi
	INVOKER	ebp+oglImports.ChoosePixelFormat

	push	ebx
	push	eax
	push	esi
	INVOKER ebp+oglImports.SetPixelFormat
%else
	push	byte 0
	push	byte 4
	push	esi
	INVOKER ebp+oglImports.SetPixelFormat
%endif

;	INVOKER	ABS(ChoosePixelFormat),esi,ebx	;dword ABS(Pixelfd)
;	INVOKER	ABS(SetPixelFormat),esi,eax,ebx	;dword ABS(Pixelfd)
;	pop	esi

	push	esi
	INVOKER	ebp+oglImports.wglCreateContext
		
	;INVOKER	ABS(wglCreateContext),esi
	xchg	eax,ebx				;ebx=hRC
	
	push	ebx
	push	esi
	INVOKER	ebp+oglImports.wglMakeCurrent	
;	INVOKER ABS(wglMakeCurrent),esi,ebx
	
	
	mov	edi,[ABS(dsound)]	
	push	byte 0
	push	dword ABS(dprimary)
	push	dword ABS(bufdesc)
	push	edi
	mov	ebx,[edi]
	call	dword [ebx+12]			;CreateSoundBuffer(dsound,&bufdesc,&dprimary,NULL)	


	mov	edi,[ABS(dprimary)]
	mov	ebx,[edi]
	push	dword ABS(waveformat)
	push	edi
	call	dword [ebx+56]			;SetFormat(dprimary,&waveformat);
	
	mov	edi,ABS(bufdesc)
	mov	eax,ABS(waveformat)
	mov	dword [eax + 4], 8363
	mov	dword [eax + 8], 8363
	mov	[edi+4],dword DSBCAPS_CTRLDEFAULT + DSBCAPS_STICKYFOCUS + DSBCAPS_STATIC
	mov	[edi+16],dword ABS(waveformat)
	
	
	xor	eax,eax				;global pointer		
	mov	ebx,ABS(dsamples)
	mov	ecx,NSAMPLES
createSampleBuffers:

	pushad	
	movzx	edx,word [eax*2+ABS(m_nBytes)]
	mov	[edi+8],edx
		
	mov	edi,[ABS(dsound)]
	push	byte 0
	push	dword ebx
	push	dword ABS(bufdesc)
	push	edi
	mov	ebx,[edi]
	call	dword [ebx+12]			;CreateSoundBuffer(dsound,&bufdesc,&samples[i],NULL)
	popad
	inc	eax
	add	ebx, byte 4	
	loop	createSampleBuffers
	
;esi = hDC
;ebx = &samples[i]
	xor	eax,eax
	mov	ebx,ABS(dsamples)	
	mov	ecx,NSAMPLES
generateSamples:
	pushad
	mov	edx,[ebx]
	mov	edi,[edx]
	push	dword DSBLOCK_ENTIREBUFFER
	push	byte 0				;ABS(trash)
	push	byte 0
	push	dword ABS(size)
	push	dword ABS(writebuf)
	push	byte 0
	push	byte 0
	push	edx
	call	dword [edi+44]			;Lock(samples[i],0,0,&writebuf,&size,NULL,&trash,DSBLOCK_ENTIREBUFFER);
	popad
		
	%include "sgen.inc"
	
	pushad
	mov	edx,[ebx]
	mov	edi,[edx]
	push	byte 0
	push	byte 0
	push	dword [ABS(size)]
	push	dword [ABS(writebuf)]
	push	edx
	call	dword [edi+76]			;Unlock(samples[i],writebuf,size,NULL,0);
	popad
	
	inc	eax
	add	ebx, byte 4
	dec	ecx
	jnz	 generateSamples
		
	
	fild	dword [ABS(cnt33k)]
	mov	edi,ABS(freqtable)
	mov	ecx,8*12
generateFreq:
	fld	st0
	fdiv	dword [ABS(cntDiv)]
	fistp	dword [edi]
	add	edi, byte 4
	fmul	dword [ABS(cnt105)]
	loop	generateFreq
	fstp	st0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;allocs i precalculs;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	push	dword	128*128*4*4
	push	byte	0
	INVOKER	ebp+oglImports.GlobalAlloc
	;INVOKER ABS(GlobalAlloc)
	mov	[ABS(flare)],eax
	
	push	dword	MAX_PARTICLES*4*2		; inclou px i vel (per aixo el *2)
	push	byte	0
	INVOKER	ebp+oglImports.GlobalAlloc
	;INVOKER ABS(GlobalAlloc)
	mov	[ABS(px)],eax

		
;	fild	word [ABS(icnt64)]
	;fchs
	mov	edi,[ABS(flare)]	
	mov	edx,128
.flare_i:	

	fild	word [ABS(icnt64)]
	;fchs
	;mov	word [ABS(j)],-64
;	add	edi,4*4
	mov	ecx,128
.flare_j:
						;j-64	i-64
;	fld	st1				;i-64	j-64	i-64
	push	edx
	fild	dword [esp]
	fisub	word [ABS(icnt64)]
	pop	edx
	fmul	st0,st0				;i*i	j-64	i-64
	
	fld	st1				;j-64	i*i	j-64	i-64
	;fild	word [ABS(j)]	
	fmul	st0,st0				;j*j	i*i	j-64	i-64
	faddp	st1,st0				;jj+ii	j	i
	fsqrt					;f	j	i

	fld	st0				;f	f	j	i
	fchs					;-f	f	j	i
	fld	st0				;-f	-f	f	j	i
	fmul	st0,st2				;-f*f	-f	f	j	i
	fmul	dword [ABS(cnt0005)]		;-f*f.5	-f	f	j	i
	
	call	fexp
	fmul	dword [ABS(cnt06)]		;ff	-f	f	j	i
				
	fxch	st2				;f	-f	ff	j	i
	fmulp	st1,st0				;-f*f	ff	j	i
	fmul	dword [ABS(cnt08)]
	call	fexp
	fmul	dword [ABS(cnt04)]		;ssi	ff	j	i
	
	fld	st1				;ff	ssi	ff	j	i
	fmul	dword [ABS(cnt07)]		;ff*.7	ssi	ff	j	i
	fadd	st0,st1				;G	ssi	ff	j	i
	fstp	dword [edi+ 4]			;ssi	ff	j	i
		
	fld	st1				;ff	ssi	G	ff	j	i
	fadd	st0,st0				;ff*2	ssi	ff	j	i	
	fadd	st0,st1				;R	ssi	ff	j	i
	fstp	dword [edi+ 0]			;ssi	ff	j	i
		
	fld	st1				;ff	ssi	ff	j	i
	fmul	dword [ABS(cnt025)]		;ff*.25	ssi	ff	j	i
	fadd	st0,st1				;B	ssi	ff	j	i
	fstp	dword [edi+ 8]			;ssi	ff	j	i	
	faddp	st1,st0				;A	j	i	
	fstp	dword [edi+12]	

	add	edi,4*4				;j	i
		
	fld1
	fsubp	st1,st0	
	loop	.flare_j
	
	fstp	st0
	
;	fld1
;	fsubp	st1,st0
	
	dec	edx
	jnz	near .flare_i	
;	fstp	st0


	mov	edi,[ABS(px)]
	mov	ecx,MAX_PARTICLES
.partPrecalc:
	fild	word [ABS(icnt40)]
	call	frand
	fisub	word [ABS(icnt40)]
	fstp	dword [edi]			;px
	
	fld1
	call	frand
	fadd	dword [ABS(cnt05)]
	fidiv	word [ABS(icnt4)]
	fstp	dword [edi+MAX_PARTICLES*4]	;vel
	add	edi,4
	loop	.partPrecalc

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;inicialitzar opengl;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;INVOKER ABS(GetTickCount)
	INVOKER	ebp+oglImports.GetTickCount
	mov	[ABS(oldTickCount)],eax
	mov	[ABS(holdrand)],eax			;yeah
	

	cmp	byte [ABS(parameter)],1
	jne	.noRand
	
	mov	ecx,24
	mov	edi,ABS(orderStuff)
.randomize:
	call	rand
	movzx	eax,al
	or	eax,eax
	jnz	.okRand
	mov	eax,32 | 2
.okRand
	stosb
	cmp	ecx,12
	jle	.nextRand
	stosb
	cmp	ecx,14	
	jle	.nextRand	
	stosb
	stosb	
.nextRand
	loop	.randomize
	
.noRand:
	push	dword GL_QUADS

	push	dword GL_COMPILE
	push	byte 1
	
	push	dword [ABS(flare)]
	push	dword GL_FLOAT
	push	dword GL_RGBA
	push	byte 0
	push	dword 128
	push	dword 128
	push	byte 4
	push	byte 0
	push	dword GL_TEXTURE_2D
	
	push	dword GL_LINEAR
	push	dword GL_TEXTURE_MIN_FILTER
	push	dword GL_TEXTURE_2D
	
	INVOKER	ebp+oglImports.glTexParameteri
	INVOKER	ebp+oglImports.glTexImage2D	
	INVOKER ebp+oglImports.glNewList	
	INVOKER ebp+oglImports.glBegin
	
	mov	eax,#2.0#
	mov	ebx,#-2.0#
	mov	edx,#1.0#
	mov	ecx,#0.0#

;- 4
	push	ecx		;0
	push	eax		;2
	push	ebx		;-2
	
	push	edx		;1
	push	ecx		;0
	
;- 3	
	push	ecx		;0
	push	eax		;2
	push	eax		;2
	
	push	edx		;1
	push	edx		;1
	
;- 2		
	push	ecx		;0
	push	ebx		;-2
	push	eax		;2
	
	push	ecx		;0
	push	edx		;1
;- 1
	push	ecx		;0
	push	ebx		;-2
	push	ebx		;-2
	
	push	ecx		;0
	push	ecx		;0
	
	INVOKER	ebp+oglImports.glTexCoord2f
	INVOKER ebp+oglImports.glVertex3f	;ABS(glVertex3f)
	INVOKER	ebp+oglImports.glTexCoord2f
	INVOKER ebp+oglImports.glVertex3f	;ABS(glVertex3f)
	INVOKER	ebp+oglImports.glTexCoord2f
	INVOKER ebp+oglImports.glVertex3f	;ABS(glVertex3f)
	INVOKER	ebp+oglImports.glTexCoord2f
	INVOKER ebp+oglImports.glVertex3f	;ABS(glVertex3f)
	
	INVOKER	ebp+oglImports.glEnd
	INVOKER	ebp+oglImports.glEndList
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;end - inicialitzar opengl;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push	dword ABS(tID)
	push	byte 0
	push	byte 0
	push	ABS(threadMain)
	push	byte 0
	push	byte 0
	INVOKER	ebp+oglImports.CreateThread
	;INVOKER ABS(CreateThread),byte 0,byte 0,dword ABS(threadMain),byte 0,byte 0,dword ABS(tID)
	mov	[ABS(thH)],eax
	
	push	dword THREAD_PRIORITY_TIME_CRITICAL
	push	eax
	INVOKER ebp+oglImports.SetThreadPriority
	;INVOKER ABS(SetThreadPriority),eax,dword THREAD_PRIORITY_TIME_CRITICAL

	;DO NOT DESTROY esi!!!!!!!!!!!!

;	INVOKER ABS(GetTickCount)
;	mov	dword [ABS(oldTickCount)],eax

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;   main   ;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


_main:	
;********************************************************************************* Update Tick Count
	INVOKER ABS(GetTickCount)
	push	eax
	sub	eax,[ABS(oldTickCount)]
	push	eax
	fild	dword [esp]	
	fidiv	word  [ABS(timeDivider)]
	fadd	dword [ABS(ts)]
	fstp	dword [ABS(ts)]	
	pop	eax
	pop	dword [ABS(oldTickCount)]
	
;********************************************************************************* Get win Messages
	xor	ebx,ebx
	mov	ebp,ABS(Msg)
	INVOKER	ABS(PeekMessage),ebp,ebx,ebx,ebx,PM_REMOVE
	mov	eax,[ebp+4]
%if	FULLSCREEN==0
	cmp	eax,WM_QUIT
	je	near exitIntro
	cmp	eax,WM_DESTROY
	je	near exitIntro
%endif
	cmp	eax,WM_KEYDOWN
	jne	.normal	
	cmp	dword [ebp+8], 0x1B
	je	near exitIntro	
.normal
;********************************************************************************* Render new frame

	mov	ebp,ABS(oglFunc)	
	pushad
	
	push	dword GL_MODELVIEW
	
	push	dword 0x40590000
	push	byte 0
	push	dword 0x3fb99999
	push	dword 0xa0000000
	push	dword 0x3ff55555
	push	dword 0x60000000
	push	dword 0x40540000
	push	byte 0
	
	push	dword GL_PROJECTION
	
	push	dword GL_POINT_SMOOTH
	
	push	dword GL_LINE_SMOOTH
	
	push	dword GL_COLOR_MATERIAL
	
	push	dword GL_LIGHT0
	
	push	dword GL_BLEND
	
	push	dword GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
	
	push	dword #1.0#
	push	byte 0
	push	byte 0
	push	dword #0.6#	
	INVOKER ebp+oglImports.glClearColor		
	INVOKER ebp+oglImports.glClear
	INVOKER ebp+oglImports.glEnable
	INVOKER ebp+oglImports.glEnable
	INVOKER ebp+oglImports.glEnable
	INVOKER	ebp+oglImports.glEnable		
	INVOKER	ebp+oglImports.glEnable
	INVOKER ebp+oglImports.glMatrixMode
	INVOKER ebp+oglImports.glLoadIdentity		
	INVOKER ebp+oglImports.gluPerspective	
	INVOKER ebp+oglImports.glMatrixMode
	INVOKER ebp+oglImports.glLoadIdentity
	

	mov	ebx,[ABS(order)]
	movzx	eax,byte [ABS(orderStuff)+ebx]
	and	eax,1
	jz	.efecte2

;--- efecte 1 	 ---------------------------------------------------------------------------------------------------------
.efecte1:
;	cmp	ebx,[ABS(lastorder)]
;	je	.noset0
	
	;movzx	eax,byte [ABS(orderStuff)+ebx-1]
	;and	eax,1
	;jnz	.noset0
	
;	mov	[ABS(lastorder)],ebx
;	mov	eax,[ABS(ts)]
;	mov	[ABS(start_ts)],eax
;.noset0:
	push	dword #5.0#
	INVOKER ebp+oglImports.glLineWidth
	
	push	byte 0
	push	dword #0.9#
	push	dword #1.0#
	INVOKER ebp+oglImports.glColor3f
	
;	INVOKER ABS(glLineWidth),#5.0#
;	INVOKER ABS(glColor3f),#1.0#,#0.9#,#0.0#

	push	dword GL_LINES
	INVOKER ebp+oglImports.glBegin
		
;	INVOKER ABS(glBegin),GL_LINES

	mov	ebx,25
.drawBackLines:

	fild	word [ABS(icnt17)]
	call	frand
	
	push	esi
	fstp	dword [esp]
	pop	esi

	push	dword #-20.0#
	push	esi
	push	dword #-30.0#
	INVOKER ebp+oglImports.glVertex3f
	
	push	dword #-20.0#
	push	esi
	push	dword # 30.0#
	INVOKER ebp+oglImports.glVertex3f
		
;	INVOKER ABS(glVertex3f),#-30.0#,esi,#-20.0#
;	INVOKER ABS(glVertex3f),# 30.0#,esi,#-20.0#
	
	dec	ebx
	jnz	.drawBackLines

	INVOKER ebp+oglImports.glEnd
		
;	INVOKER ABS(glEnd)

	push	dword GL_DEPTH_TEST
	INVOKER ebp+oglImports.glEnable
	
	push	dword GL_LIGHTING
	INVOKER ebp+oglImports.glEnable
	
;	push	dword GL_LIGHT0
;	INVOKER ebp+oglImports.glEnable
	
;	push	dword GL_COLOR_MATERIAL
;	INVOKER ebp+oglImports.glEnable
	
	push	dword #8.0#
	INVOKER ebp+oglImports.glLineWidth
	
	push	dword #0.2#
	push	byte 0
	push	dword #1.0#
	INVOKER ebp+oglImports.glColor3f
	
	push	dword GL_BLEND
	INVOKER ebp+oglImports.glDisable
	
	mov	eax,1		;x scale
	mov	ebx,1		;y scale
	xor	edx,edx		;xdispl	
	xor	edi,edi
	call	drawRubic		;drawRubic(1.f,1.f,0.f,0);		
	

	push	dword GL_LIGHTING
	INVOKER ebp+oglImports.glDisable
	
	push	dword GL_DEPTH_TEST
	INVOKER ebp+oglImports.glDisable
	
;	INVOKER ABS(glDisable),GL_LIGHTING
;	INVOKER ABS(glDisable),GL_DEPTH_TEST
		

	push	dword GL_TEXTURE_2D
	INVOKER	ebp+oglImports.glEnable
	
	push	dword GL_BLEND
	INVOKER	ebp+oglImports.glEnable
	
	push	dword GL_ONE
	push	dword GL_ONE
	INVOKER	ebp+oglImports.glBlendFunc
	
	push	dword #1.0#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor3f
	
	mov	edi,[ABS(px)]
	mov	ecx,MAX_PARTICLES
.drawParticles_ef1:
	pushad
		
	mov	esi,[edi]
	mov	ebx,#3.0#
	
	INVOKER ebp+oglImports.glLoadIdentity
	
	push	dword #-4.2#
	push	ebx
	push	esi
	INVOKER ebp+oglImports.glTranslatef
	
	
	push	byte 1
	INVOKER ebp+oglImports.glCallList
	
;	INVOKER ABS(glLoadIdentity)	
;	INVOKER ABS(glTranslatef),esi,ebx,#-4.2#
;	INVOKER ABS(glCallList),byte 1		
	
	
	push	esi
	fld	dword [esp]
	fchs
	fstp	dword [esp]
	pop	esi
	
	push	ebx
	fld	dword [esp]
	fchs
	fstp	dword [esp]
	pop	ebx	

	INVOKER ebp+oglImports.glLoadIdentity	
	;INVOKER ABS(glLoadIdentity)
	
	push	dword #-4.2#
	push	ebx
	push	esi
	INVOKER ebp+oglImports.glTranslatef
	
	push	byte 1
	INVOKER ebp+oglImports.glCallList
	
	;INVOKER ABS(glTranslatef),esi,ebx,#-4.2#
	;INVOKER ABS(glCallList),byte 1	
			
	popad
	add	di,4
	loop	.drawParticles_ef1
	
	push	dword GL_TEXTURE_2D
	INVOKER ebp+oglImports.glDisable
	
	push	dword GL_ONE_MINUS_SRC_ALPHA
	push	dword GL_SRC_ALPHA
	INVOKER	ebp+oglImports.glBlendFunc
	
	
	push	dword #8.0#
	INVOKER ebp+oglImports.glLineWidth
	
	push	byte 0
	push	byte 0
	push	byte 0
	INVOKER ebp+oglImports.glColor3f
		
	;INVOKER ABS(glDisable),GL_TEXTURE_2D
	;INVOKER ABS(glBlendFunc),GL_DST_ALPHA,GL_ONE_MINUS_SRC_ALPHA	

;	INVOKER ABS(glLineWidth),#8.0#
;	INVOKER ABS(glEnable),GL_LINE_SMOOTH
	
;	INVOKER ABS(glColor4f),#1.0#,#1.0#,#1.0#,#0.05#
	
%if 0
	mov	ebx,8
.drawChin_ef1:
	INVOKER ebp+oglImports.glLoadIdentity
	
	push	dword #-5.0#
	push	dword #2.0#
	push	dword #3.0#
	INVOKER ebp+oglImports.glTranslatef
	
	push	dword #1.0#
	push	dword #3.0#
	push	dword #3.0#
	INVOKER ebp+oglImports.glScalef
	
;	INVOKER ABS(glLoadIdentity)
;	INVOKER ABS(glTranslatef),#3.0#,#0.0#,#-5.0#
;	INVOKER ABS(glScalef),#3.f#,#3.f#,#1.f#	
	
	push	byte	0
		
	fld	dword [ABS(ts)]
	fsub	dword [ABS(start_ts)]
	fidiv	word [ABS(icnt70)]
	
	push	ebx
	fild	dword [esp]	
	fadd	st0,st0			;ts/70	t*2
	fsubp	st1,st0			;ts/70-t*2
	fld1
	fsubp	st1,st0			;... -1
	fstp	dword [esp]
		
	push	byte	0
	INVOKER ebp+oglImports.glTranslatef
	;INVOKER ABS(glTranslatef)
	
	mov	eax,ebx
	and	eax,1
	call	drawChin

	dec	ebx
	jnz	.drawChin_ef1
%endif
	
	push	dword #10.0#
	push	byte 0
	push	byte 0
	INVOKER ebp+oglImports.glTranslatef


;--- efecte 2 	 ---------------------------------------------------------------------------------------------------------	

.efecte2:
	;INVOKER ebp+oglImports.glLoadIdentity
	
	mov	ebx,[ABS(order)]
	movzx	eax,byte [ABS(orderStuff)+ebx]
	and	eax,2
	jz	.efecte3

	fld	dword [ABS(ts)]

	fidiv	word [ABS(icnt18)]
	fstp	dword [ABS(lts)]
	
	
	push	dword #-3.2#
	push	byte 0
	push	byte 0
	INVOKER ebp+oglImports.glTranslatef
	
;	INVOKER ABS(glTranslatef),#0.0#,#0.0#,#-3.2#
	
	push	dword #1.0#
	push	byte 0
	push	byte 0
	push	eax
	fld	dword [ABS(lts)]
	fimul	word [ABS(icnt10)]
	fstp	dword [esp]
	
	INVOKER ebp+oglImports.glRotatef
;	INVOKER ABS(glRotatef)
	
	push	dword GL_LIGHTING
	INVOKER ebp+oglImports.glEnable
	
;	push	dword GL_LIGHT0
;	INVOKER ebp+oglImports.glEnable
	
;	push	dword GL_COLOR_MATERIAL
;	INVOKER ebp+oglImports.glEnable

;	push	dword GL_ONE
;	push	dword GL_ONE
;	INVOKER ebp+oglImports.glBlendFunc

;	INVOKER ABS(glEnable),GL_LIGHTING
; no caldran aquestes dues
;	INVOKER ABS(glEnable),GL_LIGHT0
;	INVOKER ABS(glEnable),GL_COLOR_MATERIAL
;

	push	dword #0.2#
	push	dword #0.4#
	push	dword #0.8#
	INVOKER	ebp+oglImports.glColor3f	
	;INVOKER ABS(glColor3f),#0.8#,#0.4#,#0.2#
	
	push	dword GL_QUADS
	INVOKER	ebp+oglImports.glBegin
	;INVOKER ABS(glBegin),GL_QUADS
	
	mov	word [ABS(i)],-15
	mov	ecx,30
.y_ef2
	pushad
	
	fild	word [ABS(i)]			;i
	fmul	dword [ABS(cnt03)]		;i*.3
	fst	dword [ABS(y)]	
	fld	st0				;i*.3	i*.3
	fld	dword [ABS(lts)]			;lts	i*.3	i*.3
	fld	st0				;lts	lts	i*.3	i*.3
	faddp	st2,st0				;lts	i*.3+lts	i*.3
	fadd	dword [ABS(cnt03)]		;lts+.3	i*.3+lts	i*.3
	faddp	st2,st0				;i*.3+lts	i*.3+lts+0.3
	fcos					;cos(..)	i*.3+lts+0.3
	fstp	dword [ABS(t1)]
	fsin					;sin(...)
	fstp	dword [ABS(t2)]
	
	mov	word [ABS(j)],-15
	mov	ecx,30
.x_ef2
	pushad
	
	
	fild	word [ABS(j)]			;i
	fmul	dword [ABS(cnt03)]		;i*.3
	fst	dword [ABS(x)]
	fld	st0				;i*.3	i*.3
	fld	dword [ABS(lts)]			;lts	i*.3	i*.3
	fld	st0				;lts	lts	i*.3	i*.3
	faddp	st2,st0				;lts	i*.3+lts	i*.3
	fadd	dword [ABS(cnt03)]		;lts+.3	i*.3+lts	i*.3
	faddp	st2,st0				;i*.3+lts	i*.3+lts+0.3
	fsin					;cos(..)	i*.3+lts+0.3
	fstp	dword [ABS(k1)]
	fcos					;sin(...)
	fstp	dword [ABS(k2)]
	
	fld	dword [ABS(k1)]
	fadd	dword [ABS(t1)]
	push	eax
	fstp	dword [esp]
	push	dword [ABS(y)]
	push	dword [ABS(x)]
	INVOKER ebp+oglImports.glNormal3f
	;INVOKER ABS(glNormal3f)
	
	fld	dword [ABS(k1)]
	fadd	dword [ABS(t1)]
	push	eax
	fstp	dword [esp]
	push	dword [ABS(y)]
	push	dword [ABS(x)]
	INVOKER ebp+oglImports.glVertex3f
	;INVOKER ABS(glVertex3f)

	fld	dword [ABS(k2)]
	fadd	dword [ABS(t1)]
	push	eax
	fstp	dword [esp]
	push	dword [ABS(y)]
	fld	dword [ABS(x)]
	fadd	dword [ABS(cnt03)]
	push	eax
	fstp	dword [esp]
	INVOKER ebp+oglImports.glVertex3f
	;INVOKER ABS(glVertex3f)

	
	fld	dword [ABS(k2)]
	fadd	dword [ABS(t2)]
	push	eax
	fstp	dword [esp]
	fld	dword [ABS(y)]
	fadd	dword [ABS(cnt03)]
	push	eax
	fstp	dword [esp]		
	fld	dword [ABS(x)]
	fadd	dword [ABS(cnt03)]
	push	eax
	fstp	dword [esp]	
	INVOKER ebp+oglImports.glVertex3f
	;INVOKER ABS(glVertex3f)
	
	fld	dword [ABS(k1)]
	fadd	dword [ABS(t2)]
	push	eax
	fstp	dword [esp]
	fld	dword [ABS(y)]
	fadd	dword [ABS(cnt03)]
	push	eax
	fstp	dword [esp]		
	push	dword [ABS(x)]
	INVOKER ebp+oglImports.glVertex3f
	;INVOKER ABS(glVertex3f)

	popad
	inc	word [ABS(j)]	
	dec	ecx
	jnz	near .x_ef2
	
	popad
	inc	word [ABS(i)]
	dec	ecx
	jnz	near .y_ef2
	
	;INVOKER ABS(glEnd)	
	INVOKER ebp+oglImports.glEnd


	push	dword GL_DEPTH_TEST
	INVOKER ebp+oglImports.glDisable
	
	push	dword GL_LIGHTING
	INVOKER ebp+oglImports.glDisable
	
	;INVOKER ABS(glDisable),GL_DEPTH_TEST
	;INVOKER ABS(glDisable),GL_LIGHTING
	
	push	dword #8.0#
	INVOKER ebp+oglImports.glLineWidth
	
	push	byte 0
	push	byte 0
	push	dword #1.0#
	INVOKER ebp+oglImports.glColor3f
	
	INVOKER	ebp+oglImports.glLoadIdentity

	push	dword #-3.2#
	push	byte 0
	push	byte 0
	INVOKER ebp+oglImports.glTranslatef
	
;	INVOKER ABS(glLineWidth),#8.0#
;	INVOKER ABS(glColor3f),#1.0#,byte 0,byte 0
	
	
;	INVOKER ABS(glLoadIdentity)
;	INVOKER ABS(glTranslatef),#0.0#,#0.0#,#-3.2#
	
	
	mov	word [ABS(i)],0
	mov	ebx,3
.lines_ef2:

	push	dword GL_LINE_STRIP
	INVOKER ebp+oglImports.glBegin	
	
;	INVOKER ABS(glBegin),GL_LINE_STRIP
	

	mov	word [ABS(j)],-15	
	mov	ecx,30
.lef2_1:
	pushad
	
	fild	word [ABS(j)]
	fmul	dword [ABS(cnt03)]
	fst	dword [ABS(x)]
	fadd	dword [ABS(lts)]
	fild	word [ABS(i)]
	fimul	word [ABS(icnt30)]
	faddp	st1,st0
	fsin
	push	eax
	fst	dword [esp]		;z == k1
	
	fld	dword [ABS(x)]
	fadd	st0,st0
	fadd	dword [ABS(t1)]		;tooooommaa
	push	eax
	fstp	dword [esp]
	
		
	fiadd	word [ABS(i)]
	push	eax
	fstp	dword [esp]

	INVOKER	ebp+oglImports.glVertex3f	
	;INVOKER ABS(glVertex3f)
	

	popad
	inc	word [ABS(j)]
	loop	.lef2_1

	inc	word [ABS(i)]
	dec	ebx
	jnz	.lines_ef2
	
	INVOKER ebp+oglImports.glEnd
;	INVOKER ABS(glEnd)

;--- efecte 3 	 ---------------------------------------------------------------------------------------------------------	

.efecte3:
	mov	ebx,[ABS(order)]
	movzx	eax,byte [ABS(orderStuff)+ebx]
	and	eax,4
	jz	.efecte4

	push	dword GL_LIGHTING
	INVOKER	ebp+oglImports.glEnable
	
;	push	dword GL_LIGHT0
;	INVOKER	ebp+oglImports.glEnable
	
;	push	dword GL_COLOR_MATERIAL
;	INVOKER	ebp+oglImports.glEnable
	
;	push	dword GL_BLEND
;	INVOKER	ebp+oglImports.glEnable
	
;	INVOKER ABS(glEnable),GL_LIGHTING
	; no caldran aquestes tres (?)
;	INVOKER ABS(glEnable),GL_LIGHT0
;	INVOKER ABS(glEnable),GL_COLOR_MATERIAL
;	INVOKER ABS(glEnable),GL_BLEND
	;

	push	dword GL_DEPTH_TEST
	INVOKER	ebp+oglImports.glEnable	
	
	push	dword GL_ONE_MINUS_SRC_ALPHA
	push	dword GL_DST_ALPHA	
	INVOKER	ebp+oglImports.glBlendFunc
	
;	INVOKER ABS(glEnable),GL_DEPTH_TEST
;	INVOKER ABS(glBlendFunc),GL_DST_ALPHA,GL_ONE_MINUS_SRC_ALPHA


	push	dword #0.2#
	push	byte 0
	push	dword #1.0#
	INVOKER ebp+oglImports.glColor3f
	
	
	INVOKER ebp+oglImports.glLoadIdentity
		
	;INVOKER ABS(glColor3f),#1.0#,#0.0#,#0.2#
	;INVOKER ABS(glLoadIdentity)
	
	push	dword #1.0#
	push	byte 0
	push	byte 0
	
	fld	dword [ABS(ts)]
	fidiv	word  [ABS(icnt64)]
	fsin
	fimul	word  [ABS(timeDivider)]
	fst	dword [ABS(x)]			;per despres
	push	eax
	fstp	dword [esp]
	INVOKER	ebp+oglImports.glRotatef
	;INVOKER ABS(glRotatef)
	
	push	dword #-5.0#
	push	byte 0
	push	byte 0
	INVOKER ebp+oglImports.glTranslatef

	push	dword #1.0#
	push	dword #8.0#
	push	dword #1.0#
	INVOKER ebp+oglImports.glScalef

	push	dword #1.0#
	push	byte 0
	push	dword #1.0#
	push	dword [ABS(ts)]
	INVOKER ebp+oglImports.glRotatef
			
;	INVOKER ABS(glTranslatef),#0.0#,#0.0#,#-5.0#
;	INVOKER ABS(glScalef),#1.0#,#8.0#,#1.0#
	
	;INVOKER ABS(glRotatef),dword [ABS(ts)],#1.0#,byte 0,#1.0#
	
	call	drawCube
	
	
	push	dword GL_LIGHTING
	INVOKER	ebp+oglImports.glDisable
	
	push	dword GL_LINE
	push	dword GL_FRONT_AND_BACK
	INVOKER	ebp+oglImports.glPolygonMode
	
	push	dword #8.0#
	INVOKER	ebp+oglImports.glLineWidth
	
;	INVOKER ABS(glDisable),GL_LIGHTING
;	INVOKER ABS(glPolygonMode),GL_FRONT_AND_BACK,GL_LINE
;	INVOKER ABS(glLineWidth),#8.0#					;no se si caldria

	push	byte 0
	push	byte 0
	push	byte 0
	INVOKER	ebp+oglImports.glColor3f
	
		
;	INVOKER ABS(glColor3f),byte 0,byte 0,byte 0
	
	call	drawCube
	
	push	dword GL_FILL
	push	dword GL_FRONT_AND_BACK
	INVOKER	ebp+oglImports.glPolygonMode
	
	;INVOKER ABS(glPolygonMode),GL_FRONT_AND_BACK,GL_FILL
	

	push	dword GL_DEPTH_TEST
	INVOKER ebp+oglImports.glDisable
	
	push	dword GL_TEXTURE_2D
	INVOKER ebp+oglImports.glEnable
	
	
	push	dword #1.0#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor3f
	;INVOKER ABS(glColor3f),#1.0#,#1.0#,#1.0#
	
	push	dword GL_ONE
	push	dword GL_ONE
	INVOKER	ebp+oglImports.glBlendFunc
	
	;INVOKER ABS(glBlendFunc),GL_ONE,GL_ONE
	
	mov	edi,[ABS(px)]
	mov	ecx,MAX_PARTICLES
.part_ef3
	pushad
	INVOKER ebp+oglImports.glLoadIdentity
	
	push	dword #1.0#
	push	byte 0
	push	byte 0
	push	dword [ABS(x)]
	INVOKER	ebp+oglImports.glRotatef
	
;	INVOKER ABS(glLoadIdentity)
;	INVOKER ABS(glRotatef),dword [ABS(x)],byte 0,byte 0,#1.0#
	push	dword #-5.0#
	
	fld	dword [edi]
	fld	dword [ABS(x)]
	fidiv	word  [ABS(icnt10)]
	fsubp	st1,st0
	push	eax
	fstp	dword [esp]
	
	push	dword #-3.0#
	INVOKER	ebp+oglImports.glTranslatef
	;INVOKER ABS(glTranslatef)
	
	push	byte 1
	INVOKER	ebp+oglImports.glCallList

	push	byte 0
	push	byte 0
	push	dword #6.0#
	INVOKER	ebp+oglImports.glTranslatef	
	
	push	byte 1
	INVOKER	ebp+oglImports.glCallList
	
	
;	INVOKER ABS(glCallList),byte 1
	
	;INVOKER ABS(glTranslatef),#6.0#,byte 0,byte 0
	;INVOKER ABS(glCallList),byte 1
	
	popad
	add	edi,4	
	loop	.part_ef3
	
	push	dword GL_TEXTURE_2D
	INVOKER	ebp+oglImports.glDisable
	;INVOKER ABS(glDisable),GL_TEXTUE_2D	


;--- efecte 4 	 ---------------------------------------------------------------------------------------------------------	
.efecte4:
	mov	ebx,[ABS(order)]
	movzx	eax,byte [ABS(orderStuff)+ebx]
	and	eax,8
	jz	.efecte5


;	push	dword GL_BLEND
;	INVOKER	ebp+oglImports.glEnable
	
	push	dword GL_ONE_MINUS_SRC_ALPHA
	push	dword GL_DST_ALPHA
	INVOKER	ebp+oglImports.glBlendFunc
	
;	INVOKER ABS(glEnable),GL_BLEND
;	INVOKER ABS(glBlendFunc),GL_DST_ALPHA,GL_ONE_MINUS_SRC_ALPHA
	
;	push	dword GL_LINE_SMOOTH
;	INVOKER	ebp+oglImports.glEnable

	push	dword #2.0#
	INVOKER ebp+oglImports.glLineWidth
	
	push	byte 0
	push	dword #0.4#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor3f	
	
;	INVOKER ABS(glEnable),GL_LINE_SMOOTH
	
;	INVOKER ABS(glLineWidth),#2.0#
;	INVOKER ABS(glColor3f),#1.0#,#0.4#,byte 0
	
	
;	rotI[0]=0.f;
;	rotI[1]=0.f;
;	rotI[2]=0.f;

;	mov	esi,ABS(rotI)
;	mov	[esi+0],dword 0
;	mov	[esi+4],dword 0
;	mov	[esi+8],dword 0
		
	mov	eax,2			; x scale
	mov	ebx,3			; y scale
	mov	edx,-7			; x disp
	xor	edi,edi
	call	drawRubic

	push	dword GL_TEXTURE_2D
	INVOKER	ebp+oglImports.glEnable
	
	push	dword #1.0#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor3f	
	
;	INVOKER ABS(glEnable),GL_TEXTURE_2D
;	INVOKER ABS(glColor3f),#1.0#,#1.0#,#1.0#

	mov	edi,[ABS(px)]
	mov	word [ABS(i)],0
	mov	ecx,MAX_PARTICLES
.part_ef4:
	pushad
	
	INVOKER	ebp+oglImports.glLoadIdentity
	
	push	byte 0
	push	byte 0
	push	dword #1.5#
	INVOKER	ebp+oglImports.glTranslatef
	
	;INVOKER ABS(glLoadIdentity)
	
	;INVOKER ABS(glTranslatef),#1.5#,byte 0,byte 0
	
	push	dword #1.0#
	push	byte 0
	push	byte 0
	
	fild	word [ABS(i)]
	fsin
	fimul	word [ABS(icnt180)]
	fadd	dword [ABS(ts)]
	push	eax
	fstp	dword [esp]
	INVOKER	ebp+oglImports.glRotatef
	;INVOKER ABS(glRotatef)
	
	push	dword [edi]
	push	byte 0
	push	dword #-0.8#
	INVOKER ebp+oglImports.glTranslatef
	
	push	byte 1
	INVOKER	ebp+oglImports.glCallList
	
;	INVOKER ABS(glTranslatef),#-0.8#,byte 0,dword [edi]
;	INVOKER ABS(glCallList),byte 1

	push	byte 0
	push	byte 0
	push	dword #1.6#
	INVOKER ebp+oglImports.glTranslatef
	
	push	byte 1
	INVOKER	ebp+oglImports.glCallList
	
;	INVOKER ABS(glTranslatef),#1.6#,byte 0,byte 0
	;INVOKER ABS(glCallList),byte 1
		
	popad
	add	edi,4
	inc	word [ABS(i)]
	loop	.part_ef4
	
	push	dword GL_TEXTURE_2D
	INVOKER ebp+oglImports.glDisable
	
	;INVOKER ABS(glDisable),GL_TEXTURE_2D
	
	push	dword #1.0#
	INVOKER	ebp+oglImports.glLineWidth
	
	push	dword #0.2#
	push	dword #1.0#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor4f
	
	push	dword GL_ONE_MINUS_SRC_ALPHA
	push	dword GL_SRC_ALPHA
	INVOKER	ebp+oglImports.glBlendFunc
	
	
;	INVOKER ABS(glLineWidth),#1.0#
;	INVOKER ABS(glColor4f),#1.0#,#1.0#,#1.0#,#0.2#			;  :''''((((( .. puta guarra funcio ..
	
;	INVOKER ABS(glBlendFunc),GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA
	
	mov	edi,[ABS(px)]
	mov	word [ABS(i)],0
	mov	ecx,MAX_PARTICLES
.part_ef4_lin:
	pushad
	
	INVOKER	ebp+oglImports.glLoadIdentity
	
	push	byte 0
	push	byte 0
	push	dword #1.5#
	INVOKER	ebp+oglImports.glTranslatef
		
;	INVOKER ABS(glLoadIdentity)	
	;INVOKER ABS(glTranslatef),#1.5#,byte 0,byte 0
		
	push	dword #1.0#
	push	byte 0
	push	byte 0
	
	fild	word [ABS(i)]
	fsin
	fimul	word [ABS(icnt180)]
	fadd	dword [ABS(ts)]
	push	eax
	fstp	dword [esp]
	INVOKER	ebp+oglImports.glRotatef
	;INVOKER ABS(glRotatef)
	
	push	dword GL_LINES
	INVOKER	ebp+oglImports.glBegin
	
;	INVOKER ABS(glBegin),GL_LINES

	push	dword [edi]
	push	byte 0
	push	dword #-0.8#
	INVOKER	ebp+oglImports.glVertex3f
	
	push	dword [edi]
	push	byte 0
	push	dword # 0.8#
	INVOKER	ebp+oglImports.glVertex3f

	INVOKER	ebp+oglImports.glEnd
;	INVOKER ABS(glVertex3f),#-0.8#,byte 0,dword [edi]
;	INVOKER ABS(glVertex3f),# 0.8#,byte 0,dword [edi]
;	INVOKER ABS(glEnd)			
	popad
	add	edi,4
	inc	word [ABS(i)]
	loop	.part_ef4_lin	
	
	;INVOKER ABS(glBlendFunc),GL_DST_ALPHA,GL_ONE_MINUS_SRC_ALPHA	

;--- efecte 5 	 ---------------------------------------------------------------------------------------------------------	

.efecte5:
	mov	ebx,[ABS(order)]
	movzx	eax,byte [ABS(orderStuff)+ebx]
	and	eax,16
	jz	.efecte6

	
	fld	dword [ABS(ts)]
	fidiv	word [ABS(icnt5)]
	fstp	dword [ABS(lts)]
	
;	push	dword GL_BLEND
;	INVOKER	ebp+oglImports.glEnable
	
	;INVOKER ABS(glEnable),GL_BLEND			;	NO CALDRA
	
	push	dword #1.0#
	INVOKER	ebp+oglImports.glLineWidth
	
;	INVOKER ABS(glLineWidth),#1.0#
	
	mov	eax,9
	mov	ebx,9
	xor	edx,edx
	mov	edi,1
	call	drawRubic

	push	dword GL_ONE
	push	dword GL_ONE
	INVOKER	ebp+oglImports.glBlendFunc
	
	push	dword GL_TEXTURE_2D
	INVOKER	ebp+oglImports.glEnable

	push	dword #1.0#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor3f	
		
	;INVOKER ABS(glBlendFunc),GL_ONE,GL_ONE	
;	INVOKER ABS(glEnable),GL_TEXTURE_2D
	;INVOKER ABS(glColor3f),#1.0#,#1.0#,#1.0#

	mov	edi,[ABS(px)]
	mov	word [ABS(i)],0
	mov	ecx,MAX_PARTICLES
.part_ef5:
	pushad
	
	INVOKER ebp+oglImports.glLoadIdentity	;ABS(glLoadIdentity)
	
	push	dword #1.0#
	push	byte 0
	push	byte 0
	
	fild	word [ABS(i)]
	fsin
	fimul	word [ABS(icnt180)]
	fimul	word [ABS(icnt17)] 	; seria 16...
	fadd	dword [ABS(lts)]	
	push	eax
	fstp	dword [esp]	
	
	INVOKER	ebp+oglImports.glRotatef	
	;INVOKER ABS(glRotatef)
	
	push	dword [edi]
	push	byte 0
	push	dword [edi]
	INVOKER	ebp+oglImports.glTranslatef

	push	byte 1
	INVOKER	ebp+oglImports.glCallList
		
;	INVOKER ABS(glTranslatef),dword [edi],byte 0,dword [edi]
;	INVOKER ABS(glCallList),byte 1
	
	
	popad
	add	edi,4
	inc	word [ABS(i)]
	loop	.part_ef5
	
	push	dword GL_TEXTURE_2D
	INVOKER	ebp+oglImports.glDisable

	push	dword GL_ONE_MINUS_SRC_ALPHA
	push	dword GL_SRC_ALPHA
	INVOKER	ebp+oglImports.glBlendFunc
	
	push	dword #8.0#
	INVOKER	ebp+oglImports.glLineWidth
	
	push	dword #0.3#
	push	dword #1.0#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER ebp+oglImports.glColor4f
	
;	push	dword GL_LINE_SMOOTH
;	INVOKER	ebp+oglImports.glEnable
		
;	INVOKER ABS(glDisable),GL_TEXTURE_2D
	
;	INVOKER ABS(glBlendFunc),GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA
;	INVOKER ABS(glLineWidth),#8.0#
;	INVOKER ABS(glColor4f),#1.0#,#1.0#,#1.0#,#0.3#

;	INVOKER ABS(glEnable),GL_LINE_SMOOTH			; no caldra
	mov	word [ABS(i)],0
	mov	ebx,12
.drawChin_blur:

	INVOKER	ebp+oglImports.glLoadIdentity
	;INVOKER ABS(glLoadIdentity)
	
	
	push	dword #-5.0#
	push	byte 0
	push	byte 0
	INVOKER ebp+oglImports.glTranslatef
	
;	INVOKER ABS(glTranslatef),byte 0,byte 0,#-5.0#		


	push	dword #1.0#
	push	byte 0
	push	byte 0
	
	fld	dword [ABS(lts)]	
	fimul	word [ABS(icnt4)]
	fchs
	fsin
	fimul	word [ABS(icnt180)]
	fiadd	word [ABS(i)]
	push	eax
	fstp	dword [esp]

	INVOKER	ebp+oglImports.glRotatef
	;INVOKER ABS(glRotatef)	

	push	dword #3.0#
	push	dword #5.0#
	push	dword #3.0#
	INVOKER	ebp+oglImports.glScalef
	
;	INVOKER ABS(glScalef),#3.0#,#5.0#,#3.0#
	mov	eax,1
	call	drawChin
	
	inc	word [ABS(i)]
	dec	ebx
	jnz	.drawChin_blur
	
;--- efecte 6 	 ---------------------------------------------------------------------------------------------------------	
.efecte6:
	mov	ebx,[ABS(order)]
	movzx	eax,byte [ABS(orderStuff)+ebx]
	and	eax,32
	jz	.efecte7

	fld	dword [ABS(ts)]
	fsincos
	fimul	word [ABS(icnt5)]
	fstp	dword [ABS(k1)]
	fimul	word [ABS(icnt5)]
	fstp	dword [ABS(k2)]
	
;	call	.start_efecte
;	jmp	.end_efecte
	
;.start_efecte:
;	push	dword GL_BLEND
;	INVOKER	ebp+oglImports.glEnable
	
;	push	dword GL_LINE_SMOOTH
;	INVOKER	ebp+oglImports.glEnable
	
	push	dword GL_ONE_MINUS_SRC_ALPHA
	push	dword GL_SRC_ALPHA
	INVOKER	ebp+oglImports.glBlendFunc
	
;	INVOKER ABS(glEnable),GL_BLEND
;	INVOKER ABS(glEnable),GL_LINE_SMOOTH			; no caldra	
;	INVOKER ABS(glBlendFunc),GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA


	push	dword #8.0#
	INVOKER ebp+oglImports.glLineWidth
	
;	push	dword GL_LINE_SMOOTH
;	INVOKER ebp+oglImports.glEnable
	
	
	push	dword #0.05#
	push	dword #1.0#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor4f
	
	
;	INVOKER ABS(glColor4f),#1.0#,#1.0#,#1.0#,#0.05#
	

	mov	dword [ABS(t1)],#-4.0#
	mov	edi,ABS(k1)
	
	;mov	word [ABS(j)],1
;	mov	eax,1
	mov	ecx,2
.two:
	pushad
	
	mov	word [ABS(i)],0
	mov	ebx,50
.fin1:
	push	ecx
	INVOKER ebp+oglImports.glLoadIdentity
	;INVOKER ABS(glLoadIdentity)
	
	push	dword #1.0#
	push	byte 0
	push	byte 0
	
	fild	word [ABS(i)]
	fidiv	word [ABS(icnt8)]
	push	eax
	fstp	dword [esp]
	
	INVOKER ebp+oglImports.glRotatef
	;INVOKER ABS(glRotatef)
	
	push	dword #-10.0#
	push	dword [edi]
	push	dword [ABS(t1)]
	INVOKER ebp+oglImports.glTranslatef
	;INVOKER ABS(glTranslatef)
	
	push	dword #3.0#
	push	dword #6.0#
	push	dword #4.0#
	INVOKER ebp+oglImports.glScalef
	;INVOKER ABS(glScalef),#4.0#,#6.0#,#3.0#
	
	pop	eax
	push	eax
	dec	eax
	;movzx	eax,word [ABS(j)]
	call	drawChin
	
	pop	ecx
	inc	word [ABS(i)]
	dec	ebx
	jnz	.fin1
	
	popad
	add	edi,4		;apuntar a k2
	;dec	word [ABS(j)]
	dec	eax
	mov	dword [ABS(t1)],#4.0#
	
	loop	.two
	;dec	ecx
	;jnz	.two
	
;	ret
;.end_efecte:

	

;finalitzar ??
;--- efecte 7 	 ---------------------------------------------------------------------------------------------------------	
.efecte7:
	mov	ebx,[ABS(order)]
	movzx	eax,byte [ABS(orderStuff)+ebx]
	and	eax,64
	jz	.efecte8
	
	push	dword #1.0#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor3f
	
	push	dword GL_ONE
	push	dword GL_ONE
	INVOKER ebp+oglImports.glBlendFunc
	
	;INVOKER ABS(glColor3f),#1.0#,#1.0#,#1.0#
;	INVOKER ABS(glBlendFunc),GL_ONE,GL_ONE
	mov	edi,[ABS(px)]
	mov	word [ABS(i)],0
	
	push	dword GL_TEXTURE_2D
	INVOKER ebp+oglImports.glEnable
	
;	INVOKER ABS(glEnable),GL_TEXTURE_2D
	mov	ebx,MAX_PARTICLES
.part_ef7:
	INVOKER ebp+oglImports.glLoadIdentity
	;INVOKER ABS(glLoadIdentity)
	push	dword #1.0#
	push	byte 0
	push	byte 0
	
	fld	dword [edi]
	fidiv	word [ABS(icnt10)]
	fsin
	fimul	word [ABS(icnt180)]
	fadd	dword [ABS(ts)]
	push	eax
	fstp	dword [esp]
	INVOKER ebp+oglImports.glRotatef
;	INVOKER ABS(glRotatef)

	fld	dword [ABS(ts)]
	fiadd	word [ABS(i)]
	fidiv	word [ABS(icnt18)]
	fsin
	fimul	word [ABS(icnt8)]
	fisub	word [ABS(icnt10)]
	push	eax
	fstp	dword [esp]

;	push	dword #-10.0#
	push	byte 0
	push	dword #4.0#
	INVOKER ebp+oglImports.glTranslatef
	;INVOKER ABS(glTranslatef)
	
	push	byte 1
	INVOKER ebp+oglImports.glCallList
	;INVOKER ABS(glCallList),byte 1
	inc	word [ABS(i)]
	add	edi,4
	dec	ebx
	jnz	.part_ef7
	
	push	dword GL_TEXTURE_2D
	INVOKER ebp+oglImports.glDisable
	
	;INVOKER ABS(glDisable),GL_TEXTURE_2D

	;glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);							
	
.efecte8:
	mov	ebx,[ABS(order)]
	movzx	eax,byte [ABS(orderStuff)+ebx]
	and	eax,128
	jz	.endframe
	
	push	dword #0.1#
	push	dword #0.5#
	push	dword #1.0#
	push	dword #1.0#
	INVOKER	ebp+oglImports.glColor4f
	
	push	dword GL_ONE_MINUS_SRC_ALPHA
	push	dword GL_SRC_ALPHA
	INVOKER	ebp+oglImports.glBlendFunc

	push	dword #37.0#
	INVOKER ebp+oglImports.glPointSize
	
;	push	dword GL_POINT_SMOOTH
;	INVOKER ebp+oglImports.glEnable
		
;	INVOKER ABS(glColor4f),#1.0#,#1.0#,#0.5#,#0.1#
;	INVOKER ABS(glBlendFunc),GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA	
;	INVOKER ABS(glPointSize),#37.0#
;	INVOKER ABS(glEnable),GL_POINT_SMOOTH
	
	mov	word [ABS(i)],0
	mov	ebx,50
.fzn1:
	INVOKER ebp+oglImports.glLoadIdentity
;	INVOKER ABS(glLoadIdentity)
	
	push	dword #-15.0#
	push	dword #0.0#
	push	dword #0.0#
	INVOKER ebp+oglImports.glTranslatef
;	INVOKER ABS(glTranslatef)
	
	push	dword #1.0#
	push	byte 0
	push	byte 0
	
	fild	word [ABS(i)]
	fidiv	word [ABS(icnt8)]
	push	eax
	fstp	dword [esp]
	;INVOKER ABS(glRotatef)
	INVOKER ebp+oglImports.glRotatef
		
	push	dword #3.0#
	push	dword #6.0#
	push	dword #4.0#
	INVOKER ebp+oglImports.glScalef
	;INVOKER ABS(glScalef),#4.0#,#6.0#,#3.0#
	
	mov	eax,8
	call	drawChin
	inc	eax
	call	drawChin
	
	inc	word [ABS(i)]
	dec	ebx
	jnz	.fzn1
.endframe


; incrementador comu de particules :'D
	mov	edi,[ABS(px)]
	mov	ecx,MAX_PARTICLES
.particle_add
	fild	word [ABS(timeDivider)]
	
	fld	dword [edi]
	fadd	dword [edi+MAX_PARTICLES*4]
		
	fcomi	st0,st1				;si st0>20
	jc	.no_change
	
	fxch	st1
	fchs
		
.no_change:	 		
	
;	push	eax
;	fist	dword [esp]
	;fist	word  [ABS(temp)]
	fstp	dword [edi]
	fstp	st0

;	pop	eax
;	cmp	eax,20
	;cmp	word [ABS(temp)],20
;	jl	.okPx
	
;	mov	dword [edi],#-20.0#
;.okPx:
	add	edi,4
	loop	.particle_add
	
	popad
	push	esi
	INVOKER	ebp+oglImports.wglSwapBuffers
	jmp	near _main
	


;********************************************************************************* Exit Intro
exitIntro:
%if FULLSCREEN
	INVOKER ABS(ChangeDisplaySettings),byte 0,byte 0
	INVOKER ABS(ShowCursor),byte 1
%endif	
	INVOKER	ABS(ExitProcess),byte 0
						; no cal pop ni ret, aqui no ha d'arribar mai!


fexp:
	fldl2e
	fmulp	st1,st0
	fld1
	fld	st1
	fprem
	f2xm1
	faddp	st1,st0
	fscale
	fxch	st1
	fstp	st0
	ret


rand:
	mov	eax,[ABS(holdrand)]
	imul	eax,214013
	add	eax,2531011
	mov	[ABS(holdrand)],eax
	and	eax,0x7FFF
	ret

	;st0	-> max
frand:
	call	rand
	
	push	eax
	fild	dword [esp]		;rand()		max
	pop	eax
	
	fidiv	word [ABS(icnt32k)]
	fsub	dword [ABS(cnt05)]
	fmulp	st1,st0
	ret
	
	
drawCube:

	xor	ebx,ebx
	mov	eax,# 1.0#
	mov	edx,#-1.0#
;6
	push	edx
	push	eax
	push	eax
	
	push	eax
	push	eax
	push	eax
		
	push	eax
	push	eax
	push	edx
	
	push	edx
	push	eax
	push	edx
	
	push	ebx
	push	eax
	push	ebx
	
;5
	push	edx
	push	edx
	push	eax
	
	push	eax
	push	edx
	push	eax
		
	push	eax
	push	edx
	push	edx
	
	push	edx
	push	edx
	push	edx
	
	push	ebx
	push	edx
	push	ebx

;4
	push	eax
	push	eax
	push	edx
	
	push	eax
	push	eax
	push	eax
		
	push	eax
	push	edx
	push	eax
	
	
	push	eax
	push	edx
	push	edx
	
	push	eax
	push	ebx
	push	ebx


;3 	
	push	eax
	push	edx
	push	edx
	
	push	eax
	push	eax
	push	edx
		
	push	edx
	push	eax
	push	edx
	
	
	push	edx
	push	edx
	push	edx
	
	push	ebx
	push	ebx
	push	edx

	
;2 	
	push	eax
	push	edx
	push	eax
	
	push	eax
	push	eax
	push	eax
		
	push	edx
	push	eax
	push	eax
	
	
	push	edx
	push	edx
	push	eax
	
	push	ebx
	push	ebx
	push	eax
	
;1
	
	push	edx
	push	eax
	push	edx
	
	push	edx
	push	eax
	push	eax
		
	push	edx
	push	edx
	push	eax
	
	
	push	edx
	push	edx
	push	edx
	
	push	edx
	push	ebx
	push	ebx
	push	dword	GL_QUADS
	
	INVOKER ebp+oglImports.glBegin
	;INVOKER ABS(glBegin)
	
	mov	ebx,6
.drawcube_loop:
	INVOKER ebp+oglImports.glNormal3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
%if 0	
	INVOKER ebp+oglImports.glNormal3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	
	INVOKER ebp+oglImports.glNormal3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	
	INVOKER ebp+oglImports.glNormal3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	
	INVOKER ebp+oglImports.glNormal3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	
	INVOKER ebp+oglImports.glNormal3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
	INVOKER ebp+oglImports.glVertex3f
%endif	
	dec	ebx
	jnz	.drawcube_loop

	INVOKER ebp+oglImports.glEnd
	
	ret
	

;eax == scale x,y
;edx == x offset
drawRubic:

	push	eax	
	fild	dword [esp]
	pop	eax
	fstp	dword [ABS(k1)]
	
	push	ebx	
	fild	dword [esp]
	pop	ebx
	fstp	dword [ABS(k2)]
	
	push	edx
	fild	dword [esp]
	pop	edx
	fisub	word [ABS(icnt5)]
	fstp	dword [ABS(t1)]
	
	
	xor	ecx,ecx	
.rubicOutter:
	pushad
	
	or	edi,edi
	jnz	near .nextLoopRubic
	;mirar ini_t
	
	mov	word [ABS(k)],-1
	mov	ecx,3
.rubicZ:
	pushad
	
	mov	word [ABS(j)],-1
	mov	ecx,3
.rubicY:
	pushad
	
	mov	word [ABS(i)],-1
	mov	ecx,3
.rubicX:
	pushad

	INVOKER ebp+oglImports.glLoadIdentity	
	;INVOKER ABS(glLoadIdentity)
	
	push	dword #-10.0#
	push	byte 0
	push	dword [ABS(t1)]
	INVOKER	ebp+oglImports.glTranslatef
	
;	INVOKER ABS(glTranslatef),dword [ABS(t1)],#0.0#,#-10.0#		;falta xdesp

	push	dword #1.0#
	push	dword [ABS(k2)]
	push	dword [ABS(k1)]
	INVOKER	ebp+oglImports.glScalef
		
	;INVOKER ABS(glScalef),dword [ABS(k1)],dword [ABS(k2)],#1.0#
	
	push	dword #1.0#
	push	dword #1.0#
	push	byte 0
	push	dword [ABS(ts)]
	INVOKER ebp+oglImports.glRotatef
	
	
	;INVOKER ABS(glRotatef),dword [ABS(ts)],byte 0,#1.0#,#1.0#
	;INVOKER ABS(glRotatef),rotI[rotpos],#1.0#,byte 0,byte 0
	
;	push	byte 0
;	push	byte 0
;	push	dword #1.0#
;	mov	eax,[ABS(rotpos)]
;	push	dword [ABS(rotI)+eax)]
;	INVOKER	ebp+oglImports.glRotatef
	
	fild	word [ABS(k)]
	fmul	dword [ABS(cnt2d2)]	
	push	eax
	fstp	dword [esp]		;z
	
	fild	word [ABS(j)]
	fmul	dword [ABS(cnt2d2)]
	push	eax
	fstp	dword [esp]		;y
	
	fild	word [ABS(i)]
	fmul	dword [ABS(cnt2d2)]
	push	eax
	fstp	dword [esp]		;x
		
	INVOKER ebp+oglImports.glTranslatef
	;INVOKER ABS(glTranslatef)
	
	call	drawCube

	popad
	inc	word [ABS(i)]
	dec	ecx
	jnz	near .rubicX	
	
	popad
	inc	word [ABS(j)]
	dec	ecx
	jnz	near .rubicY	
	
	popad
	inc	word [ABS(k)]
	dec	ecx
	jnz	near .rubicZ	
	
.nextLoopRubic

	push	byte 0
	push	byte 0
	push	byte 0
	
	push	dword GL_LINE
	push	dword GL_FRONT_AND_BACK
	INVOKER ebp+oglImports.glPolygonMode
	INVOKER	ebp+oglImports.glColor3f	
	popad
	xor	edi,edi
	inc	ecx
	cmp	ecx,2
	jne	near .rubicOutter

	
	push	dword GL_FILL
	push	dword GL_FRONT_AND_BACK
	INVOKER ebp+oglImports.glPolygonMode
	
	;INVOKER ABS(glPolygonMode),GL_FRONT_AND_BACK,GL_FILL
	ret

;eax: which char
drawChin:
	pushad
	mov	esi,ABS(chin1)
	mov	ebx,10*2
	
	or	eax,eax
	jnz	.okChinChar
	
	mov	esi,ABS(chin2)
	mov	ebx,7*2
	jmp	.noFuzz
.okChinChar:
	cmp	eax,8
	jl	.noFuzz
	
	mov	esi,ABS(fznchar)
	mov	ebx,3*2
	
.noFuzz
	mov	edx,GL_POINTS
	cmp	eax,9
	je	.doStart
	mov	edx,GL_LINES
.doStart:
	push	edx
	INVOKER ebp+oglImports.glBegin
	;INVOKER ABS(glBegin),edx
.bucle_chinChar:
		
	push	byte 0		;z
	
	lodsw
	
	sub	al,128
	sub	ah,128
	
	movsx	edx,al		;edx=y/2 (int)
	movsx	eax,ah		;eax=x/2 (int)
	
	add	edx,edx
	add	eax,eax
	
	push	eax
	fild	dword [esp]
	fchs
	fidiv	word [ABS(icnt256)]
	fstp	dword [esp]	;y
	
	push	edx
	fild	dword [esp]
	fidiv	word [ABS(icnt256)]
	fstp	dword [esp]	;x
	
	INVOKER ebp+oglImports.glVertex3f
	;INVOKER ABS(glVertex3f)

	dec	ebx
	jnz	.bucle_chinChar

	INVOKER ebp+oglImports.glEnd
;	INVOKER ABS(glEnd)

	popad
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;wnd proc - no fa res;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
%include "inner_pl.inc"

	editName	db "EDIT",0	

	vols		dd	-10000,-800,-700,0
	cnt105		dd	1.05946309436
	cnt33k		dd	33152
	cntDiv		dd	127.71542846
	cnt127		dw	127
		
	ts		dd	0	
	timeDivider	dw	20

	cnt0005		dd	0.005
	cnt025		dd	0.25
	cnt03		dd	0.3
	cnt04		dd	0.4
	cnt05		dd	0.5
	cnt06		dd	0.6
	cnt08		dd	0.8
	cnt07		dd	0.7	
;	cnt1d9		dd	1.9
	cnt2d2		dd	2.2		
		
	icnt4		dw	4		;-20
	icnt5		dw	5		;-18
	icnt8		dw	8		;-16
	icnt10		dw	10		;-14
	icnt17		dw	17		;-12
	icnt18		dw	18 ; ..		;-10
;	icnt20		dw	20		;-8
	icnt30		dw	30		;-4
	icnt40		dw	40		;-2
	icnt64		dw	64		;0
	icnt70		dw	70		;2
	icnt180		dw	180		;4
	icnt256		dw	256		;6
	icnt32k		dw	32000		;8
	icnt2250	dw	2250		;10
	
%include "chin_char.inc"	
%include "samples.inc"
%include "zik.asm"
%include "imports.inc"
%include "orderstuff.inc"

;section .bss
	Msg		resd	7
	dsound		resd	1
	dprimary	resd	1
	
	dsamples	resd	20	;NSAMPLES			;6 samples

	oldTickCount	resd	1			
	size		resd	1
	writebuf	resd	1
	wTemp		resd	1
	thH		resd	1
	tID		resd	1
	
	freqtable	resd	8*12		
	dmScreen	resd	37
	
	order		resd	1	
	row		resd	1
	
	i		resw	1
	j		resw	1
	k		resw	1

	;temp		resd	1	
	flare		resd	1
	px		resd	1
	
	k1		resd	1
	k2		resd	1
	t1		resd	1
	t2		resd	1
	x		resd	1
	y		resd	1

;	lastorder	resd	1	
;	start_ts	resd	1	
	lts		resd	1
	holdrand	resd	1
_OffImage:
_OffModule:
