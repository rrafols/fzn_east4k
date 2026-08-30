%macro START_PROGRAM 0
align 512, db 0
%push Code
_Code:
%endmacro

%macro IMPORT_DLL 3
%ifnctx Imports
	_Imports:
	%push Imports
	align 4, db 0
%endif
			dd %2		; Ptr to original "thunk"
			dd 0		; Time stamp
			dd 0		; Forwarder chain
			dd %1		; Ptr to DLL name
			dd %3		; Ptr to replaced "thunk"
%endmacro

%macro END_IMPORT_DLL 0
%pop
times 5 dd 0
_OffImports:
%endmacro

%macro IMPORT_FUNCTION 2
align 2, db 0
%1:			dw 0		; Hint
			db %2		; Function name
%endmacro

%macro INVOKER 1-*
   %rep %0-1
      %rotate -1
      push %1
   %endrep
   %rotate -1
   call dword [%1]
%endmacro

%macro INVOKE 1-*
   %rep %0-1
      %rotate -1
      push %1
   %endrep
   %rotate -1
   call dword [ABS(%1)]
%endmacro

%macro INVOKEp 1-*
   pushad
   %rep %0-1
      %rotate -1
      push %1
   %endrep
   %rotate -1
   call dword [ABS(%1)]
   popad
%endmacro