# Microsoft Developer Studio Project File - Name="libcep_feat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libcep_feat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libcep_feat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libcep_feat.mak" CFG="libcep_feat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libcep_feat - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libcep_feat - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libcep_feat - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libcep_feat - Win32 Debug"

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

# Name "libcep_feat - Win32 Release"
# Name "libcep_feat - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\agc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\agc_emax.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\agc_max.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\cep_frame.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\cmn.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\dcep_frame.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\ddcep_frame.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\del_sil_seg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\live_norm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\norm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\r_agc_noise.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\s2_cep.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\s2_dcep.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\s2_ddcep.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\s2_feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\silcomp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v1_feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v2_feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v3_feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v4_feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v5_feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v6_feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v7_feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v8_feat.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\agc_emax.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\agc_max.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\cep_frame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\cep_manip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\dcep_frame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\ddcep_frame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\del_sil_seg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\feat_config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\live_norm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\norm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\r_agc_noise.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\s2_cep.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\s2_dcep.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\s2_ddcep.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\s2_feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v1_feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v2_feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v3_feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v4_feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v5_feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v6_feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v7_feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcep_feat\v8_feat.h
# End Source File
# End Group
# End Target
# End Project
