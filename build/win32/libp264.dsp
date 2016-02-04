# Microsoft Developer Studio Project File - Name="libp264" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libp264 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libp264.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libp264.mak" CFG="libp264 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libp264 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libp264 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /Ob2 /I "./core" /I "./encode" /I "./decode" /I "../../extras" /I "../.." /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D "__P264__" /D "HAVE_MMXEXT" /D "ARCH_X86" /D "HAVE_STDINT_H" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"bin/libp264.lib"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "./core" /I "./encode" /I "./decode" /I "../../extras" /I "../.." /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D "__P264__" /D "HAVE_MMXEXT" /D "ARCH_X86" /D "HAVE_STDINT_H" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"bin/libp264.lib"

!ENDIF 

# Begin Target

# Name "libp264 - Win32 Release"
# Name "libp264 - Win32 Debug"
# Begin Group "core"

# PROP Default_Filter ".c;.h;"
# Begin Group "core_h"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\..\core\bs.h
# End Source File
# Begin Source File

SOURCE=..\..\core\cabac.h
# End Source File
# Begin Source File

SOURCE=..\..\core\clip1.h
# End Source File
# Begin Source File

SOURCE=..\..\core\core.h
# End Source File
# Begin Source File

SOURCE=..\..\core\cpu.h
# End Source File
# Begin Source File

SOURCE=..\..\core\csp.h
# End Source File
# Begin Source File

SOURCE=..\..\core\dct.h
# End Source File
# Begin Source File

SOURCE=..\..\core\frame.h
# End Source File
# Begin Source File

SOURCE=..\..\core\macroblock.h
# End Source File
# Begin Source File

SOURCE=..\..\core\mc.h
# End Source File
# Begin Source File

SOURCE=..\..\core\pixel.h
# End Source File
# Begin Source File

SOURCE=..\..\core\predict.h
# End Source File
# Begin Source File

SOURCE=..\..\core\quant.h
# End Source File
# Begin Source File

SOURCE=..\..\core\set.h
# End Source File
# Begin Source File

SOURCE=..\..\core\vlc.h
# End Source File
# End Group
# Begin Group "I386"

# PROP Default_Filter "*.h,*.c,*-a.asm"
# Begin Source File

SOURCE="..\..\core\i386\cpu-a.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\cpu-a.asm"
InputName=cpu-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Debug
InputPath="..\..\core\i386\cpu-a.asm"
InputName=cpu-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\core\i386\dct-a.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\dct-a.asm"
InputName=dct-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Debug
InputPath="..\..\core\i386\dct-a.asm"
InputName=dct-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\core\i386\dct-c.c"
# End Source File
# Begin Source File

SOURCE=..\..\core\i386\dct.h
# End Source File
# Begin Source File

SOURCE="..\..\core\i386\deblock-a.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\deblock-a.asm"
InputName=deblock-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Debug
InputPath="..\..\core\i386\deblock-a.asm"
InputName=deblock-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\core\i386\mc-a.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\mc-a.asm"
InputName=mc-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Debug
InputPath="..\..\core\i386\mc-a.asm"
InputName=mc-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\core\i386\mc-a2.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\mc-a2.asm"
InputName=mc-a2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Debug
InputPath="..\..\core\i386\mc-a2.asm"
InputName=mc-a2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\core\i386\pixel-a.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\pixel-a.asm"
InputName=pixel-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Debug
InputPath="..\..\core\i386\pixel-a.asm"
InputName=pixel-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\core\i386\pixel-sse2.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\pixel-sse2.asm"
InputName=pixel-sse2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\i386\pixel.h
# End Source File
# Begin Source File

SOURCE="..\..\core\i386\predict-a.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\predict-a.asm"
InputName=predict-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Debug
InputPath="..\..\core\i386\predict-a.asm"
InputName=predict-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\core\i386\quant-a.asm"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Release
InputPath="..\..\core\i386\quant-a.asm"
InputName=quant-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# Begin Custom Build - Assembly $(InputPath)
IntDir=.\Debug
InputPath="..\..\core\i386\quant-a.asm"
InputName=quant-a

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\..\core\cabac.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\core.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\cpu.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\csp.c
# End Source File
# Begin Source File

SOURCE=..\..\core\dct.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\frame.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\macroblock.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\core\i386\mc-c.c"

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\mc.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\mdate.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\pixel.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\predict.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/core_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/core_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\core\quant.c
# End Source File
# Begin Source File

SOURCE=..\..\core\set.c
# End Source File
# End Group
# Begin Group "decoder"

# PROP Default_Filter ".c"
# Begin Group "dec_h"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\..\decoder\dec_cavlc.h
# End Source File
# Begin Source File

SOURCE=..\..\decoder\lists.h
# End Source File
# Begin Source File

SOURCE=..\..\decoder\macroblock.h

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/dec_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/dec_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\decoder\set.h

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/dec_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/dec_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\decoder\vlc.h

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/dec_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/dec_debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\..\decoder\dec_cavlc.c
# End Source File
# Begin Source File

SOURCE=..\..\decoder\decoder.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/dec_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/dec_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\decoder\lists.c
# End Source File
# Begin Source File

SOURCE=..\..\decoder\macroblock.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/dec_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/dec_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\decoder\set.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/dec_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/dec_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\decoder\vlc.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/dec_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/dec_debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "extras"

# PROP Default_Filter ".c"
# Begin Group "extras_h"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\..\extras\getopt.h

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/util_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/util_debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\extras\stdint.h

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/util_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/util_debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\..\extras\getopt.c

!IF  "$(CFG)" == "libp264 - Win32 Release"

# PROP Intermediate_Dir "obj/util_release"

!ELSEIF  "$(CFG)" == "libp264 - Win32 Debug"

# PROP Intermediate_Dir "obj/util_debug"

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
