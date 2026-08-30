;-- Imports -----------------------------------------------------------------
align 4, db 0
IMPORT_DLL _User32Name-_DOSH, _User32Pointers, _User32Pointers
IMPORT_DLL _Kernel32Name-_DOSH, _Kernel32Pointers, _Kernel32Pointers
IMPORT_DLL _OpenGL32Name, _OpenGL32Pointers, _OpenGL32Pointers
IMPORT_DLL _glu32Name, _glu32Pointers, _glu32Pointers
IMPORT_DLL _DSoundName, _dsoundPointers, _dsoundPointers
IMPORT_DLL _gdi32Name, _gdi32Pointers, _gdi32Pointers
END_IMPORT_DLL


_Kernel32Pointers:
oglFunc:
ExitProcess		dd _ExitProcess
CreateThread		dd _CreateThread
SetThreadPriority	dd _SetThreadPriority
WaitForSingleObject	dd _WaitForSingleObject
GetTickCount		dd _GetTickCount
GlobalAlloc		dd _GlobalAlloc
			dd 0
_User32Pointers:
CreateWindowEx		dd _CreateWindowEx-_DOSH
PeekMessage		dd _PeekMessage
;DefWindowProc		dd _DefWindowProc
GetDC			dd _GetDC
ChangeDisplaySettings	dd _ChangeDisplaySettings
ShowCursor		dd _ShowCursor
			dd 0
			
_OpenGL32Pointers:
wglMakeCurrent		dd _wglMakeCurrent-_DOSH
wglCreateContext	dd _wglCreateContext
wglSwapBuffers		dd _wglSwapBuffers


glClear			dd _glClear
glClearColor		dd _glClearColor
glEnable		dd _glEnable
glDisable		dd _glDisable
glBlendFunc		dd _glBlendFunc
glMatrixMode		dd _glMatrixMode
glLoadIdentity		dd _glLoadIdentity
glTexImage2D		dd _glTexImage2D
glTexParameteri		dd _glTexParameteri
glBegin			dd _glBegin
glEnd			dd _glEnd
glNewList		dd _glNewList
glEndList		dd _glEndList
glCallList		dd _glCallList
glTexCoord2f		dd _glTexCoord2f
glVertex3f		dd _glVertex3f
glNormal3f		dd _glNormal3f
glPolygonMode		dd _glPolygonMode
glColor3f		dd _glColor3f
glColor4f		dd _glColor4f
glTranslatef		dd _glTranslatef
glRotatef		dd _glRotatef
glLineWidth		dd _glLineWidth
glScalef		dd _glScalef
glPointSize		dd _glPointSize
			dd  0
			
_glu32Pointers:
gluPerspective		dd _gluPerspective
			dd 0	

_dsoundPointers:
DirectSoundCreate	dd _DirectSoundCreate
			dd 0

_gdi32Pointers:
ChoosePixelFormat	dd _ChoosePixelFormat
SetPixelFormat		dd _SetPixelFormat
			dd 0

			dd 0

	IMPORT_FUNCTION	_ExitProcess,		'ExitProcess'
	IMPORT_FUNCTION	_CreateThread,		'CreateThread'
	IMPORT_FUNCTION	_SetThreadPriority,	'SetThreadPriority'
	IMPORT_FUNCTION _WaitForSingleObject,	'WaitForSingleObject'
	IMPORT_FUNCTION _GetTickCount,		'GetTickCount'
	IMPORT_FUNCTION _GlobalAlloc,		'GlobalAlloc'
		
	IMPORT_FUNCTION	_PeekMessage,		'PeekMessageA'
;	IMPORT_FUNCTION	_DefWindowProc,		'DefWindowProcA'
	IMPORT_FUNCTION	_GetDC,			'GetDC'
	IMPORT_FUNCTION _ChangeDisplaySettings, 'ChangeDisplaySettingsA'	
	IMPORT_FUNCTION _ShowCursor,		'ShowCursor'
	
	IMPORT_FUNCTION	_wglCreateContext,	'wglCreateContext'
	IMPORT_FUNCTION	_wglSwapBuffers,	'wglSwapBuffers'	

	IMPORT_FUNCTION _glClear,		'glClear'
	IMPORT_FUNCTION _glClearColor,		'glClearColor'
	IMPORT_FUNCTION _glEnable,		'glEnable'
	IMPORT_FUNCTION _glDisable,		'glDisable'
	IMPORT_FUNCTION _glBlendFunc,		'glBlendFunc'
	IMPORT_FUNCTION _glMatrixMode,		'glMatrixMode'
	IMPORT_FUNCTION _glLoadIdentity,	'glLoadIdentity'
	IMPORT_FUNCTION _glTexImage2D,		'glTexImage2D'	
	IMPORT_FUNCTION _glTexParameteri,	'glTexParameteri'
	IMPORT_FUNCTION _glBegin,		'glBegin'
	IMPORT_FUNCTION	_glEnd,			'glEnd'	
	IMPORT_FUNCTION _glNewList,		'glNewList'
	IMPORT_FUNCTION _glEndList,		'glEndList'
	IMPORT_FUNCTION _glCallList,		'glCallList'
	IMPORT_FUNCTION _glTexCoord2f,		'glTexCoord2f'
	IMPORT_FUNCTION _glVertex3f,		'glVertex3f'
	IMPORT_FUNCTION _glNormal3f,		'glNormal3f'
	IMPORT_FUNCTION _glPolygonMode,		'glPolygonMode'
	IMPORT_FUNCTION _glColor3f,		'glColor3f'
	IMPORT_FUNCTION _glColor4f,		'glColor4f'
	IMPORT_FUNCTION _glTranslatef,		'glTranslatef'
	IMPORT_FUNCTION _glRotatef,		'glRotatef'
	IMPORT_FUNCTION _glLineWidth,		'glLineWidth'
	IMPORT_FUNCTION _glScalef,		'glScalef'
	IMPORT_FUNCTION _glPointSize,		'glPointSize'
	
	IMPORT_FUNCTION _gluPerspective,	'gluPerspective'	
	
	IMPORT_FUNCTION _DirectSoundCreate,	'DirectSoundCreate'	
	
	IMPORT_FUNCTION _ChoosePixelFormat,	'ChoosePixelFormat'
	IMPORT_FUNCTION _SetPixelFormat,	'SetPixelFormat'
	db	0
	
	struc	oglImports
		.ExitProcess		resd	1
		.CreateThread		resd	1
		.SetThreadPriority	resd	1
		.WaitForSingleObject	resd	1
		.GetTickCount		resd	1
		.GlobalAlloc		resd	1
		.reserved0		resd	1
		.CreateWindowEx		resd	1
		.PeekMessage		resd	1
		.GetDC			resd	1
		.ChangeDisplaySettings	resd	1
		.ShowCursor		resd	1
		.reserved1		resd	1
		.wglMakeCurrent		resd	1
		.wglCreateContext	resd	1
		.wglSwapBuffers		resd	1
		.glClear		resd	1
		.glClearColor		resd	1
		.glEnable		resd	1
		.glDisable		resd	1
		.glBlendFunc		resd	1
		.glMatrixMode		resd	1
		.glLoadIdentity		resd	1
		.glTexImage2D		resd	1
		.glTexParameteri	resd	1
		.glBegin		resd	1
		.glEnd			resd	1
		.glNewList		resd	1
		.glEndList		resd	1
		.glCallList		resd	1
		.glTexCoord2f		resd	1
		.glVertex3f		resd	1
		.glNormal3f		resd	1
		.glPolygonMode		resd	1
		.glColor3f		resd	1
		.glColor4f		resd	1
		.glTranslatef		resd	1
		.glRotatef		resd	1
		.glLineWidth		resd	1
		.glScalef		resd	1
		.glPointSize		resd	1
		.reserved2		resd	1
		.gluPerspective		resd	1
		.reserved3		resd	1		
		.DirectSoundCreate	resd	1
		.reserved4		resd	1		
		.ChoosePixelFormat	resd	1
		.SetPixelFormat		resd	1
		.reserved5		resd	1
	endstruc