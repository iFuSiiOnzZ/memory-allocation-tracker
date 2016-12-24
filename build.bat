@ECHO OFF

:: Create folders if not exist
IF NOT EXIST ..\Bin MKDIR ..\Bin
IF NOT EXIST ..\Bin\App MKDIR ..\Bin\App
IF NOT EXIST ..\Bin\Intermediate MKDIR ..\Bin\Intermediate

:: Set paths
SET ExeName=DebugMemory
SET ExePath=..\Bin\App\%ExeName%

SET PDBFiles=..\Bin\App\
SET IntermediatePath=..\Bin\Intermediate\

:: Delete files from paths
DEL /Q /S %IntermediatePath%*.* 1>NUL
DEL /Q %PDBFiles%*.pdb 2>NUL

DEL /Q %PDBFiles%*.Exe 2>NUL
DEL /Q %PDBFiles%*.ilk 2>NUL


:: Compiler flags
SET CommonCompilerFlags= /nologo /Od /Z7 /W4 /Fd%PDBFiles% /Fo%IntermediatePath% /Fe%ExePath%
SET Defines=-D_DEBUG -DWIN64

:: Linker flags
SET CommonLinkerFlags= User32.lib Kernel32.lib

:: Compile
call cl %CommonCompilerFlags% %Defines% build.cpp /link %CommonLinkerFlags%