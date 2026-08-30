rem Build the original Win32 intro.
rem fuxnasm rewrites the #1.0# float literals, then nasm assembles.
rem The data tables come from ..\..\..\common\data, shared with the other ports.

if not exist ..\build mkdir ..\build
fuxnasm <base.asm >..\build\intro.asm
nasmw ..\build\intro.asm -i..\..\..\common\data\ -E ..\build\error.log -O3 -o ..\build\intro.exe
type ..\build\error.log
