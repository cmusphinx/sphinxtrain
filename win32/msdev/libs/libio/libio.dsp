# Microsoft Developer Studio Project File - Name="libio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libio - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libio.mak" CFG="libio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libio - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libio - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libio - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\lib\Release"
# PROP Intermediate_Dir "..\..\..\..\lib\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libio - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\lib\Debug"
# PROP Intermediate_Dir "..\..\..\..\lib\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libio - Win32 Release"
# Name "libio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\corpus.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\fgets_wo_nl.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\fp_cache.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\fread_retry.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\model_def_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\read_line.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3_open.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3cb2mllr_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3gau_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3lamb_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3map_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3mixw_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3regmat_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3tmat_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\s3ts2cb_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\segdmp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\swap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\topo_read.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libio\uttfile.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
