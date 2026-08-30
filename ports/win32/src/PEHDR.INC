
bits 32
org 0E00h ; 200h bytes of header followed by code relocated to 1000h

%define UNIMPORTANT 0
%define _ImageBase 00400000h
%define ABS(x) ((x)+_ImageBase)
%define HABS(x) ((x)+_ImageBase-_DOSH)

;-- DOS header --------------------------------------------------------------
_DOSH:			dw 'MZ' 	; DOS header magic number -> 58 bytes of useful stuff residing in the DOS header ;-)
_User32Name		db 'USER32.DLL',0
_Kernel32Name		db 'KERNEL32.DLL',0
_wglMakeCurrent		db 0, 0, 'wglMakeCurrent'
_CreateWindowEx 	db 0, 0, 'CreateWindowExA', 0
			dd _PEH-_DOSH	; Offset of PE header

;-- PE header ---------------------------------------------------------------

_PEH:			dd 'PE' 	; PE header magic number
			dw 014Ch	; i386 machine
			dw 1		; 1 section
			dd UNIMPORTANT	; Time stamp
			dd UNIMPORTANT	; Ptr to symbol table
			dd 0		; Number of symbols
			dw _SectionH-_OptionalH ; Size of optional header
			dw 010Fh	; Characteristics: 32-bit executable

;-- Optional header ---------------------------------------------------------

_OptionalH:		dw 010Bh	; Optional header magic number
			dw UNIMPORTANT	; Linker version
			dd _OffModule-1000h ; Size of code
			dd 0		; Size of initialized data
			dd 0		; Size of uninitialized data
			dd _EntryPoint	; Ptr to entry point
			dd _Code	; Base of code
			dd UNIMPORTANT	; Base of data
			dd _ImageBase	; Image base
			dd 1000h	; Section alignment
			dd 200h 	; File alignment	;200h
			dd 4		; OS version (4.00)
			dd 0		; Image version (0.00)
			dd 4		; Subsystem version (4.00)
			dd UNIMPORTANT	; Win32 version
			dd _OffImage	; Image size
			dd _Code-_DOSH	; Headers size
			dd UNIMPORTANT	; Checksum
			dw 2		; Subsystem (Win32 GUI)
			dw UNIMPORTANT	; DLL characteristics
			dd 100000h	; Stack size
			dd 1000h	; Stack commit
			dd 100000h	; Heap size
			dd 1000h	; Heap commit
			dd 0		; Loader flags
			dd 16		; Number of directories
					; ...i wish there were only 2 :-(
;-- Directories -------------------------------------------------------------

_ExportsDirectory:	dd UNIMPORTANT	; Ptr to exports section
			dd 0		; Size of exports section
_ImportsDirectory:	dd _Imports	; Ptr to imports section
			dd _OffImports-_Imports ; Size of imports section
; The following hurts a lot :-(
	     times 14*2 dd 0		; Other directories are empty

;-- Program section ---------------------------------------------------------

_SectionH:		dd 'fzn03'	; Section name			
			dd _OffModule-1000h ; Virtual size
			dd _Code	; Virtual address
			dd _OffImage-_Code ; Raw data size
			dd _Code-_DOSH	; Ptr to raw data
			dd UNIMPORTANT	; Ptr to relocations
			dd UNIMPORTANT	; Ptr to line numbers
			dw 0		; Number of relocations
			dw 0		; Number of line numbers
			dd 0E0000060h	; Flags: code, data, execute, read, write

%undef UNIMPORTANT
