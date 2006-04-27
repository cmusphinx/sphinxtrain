# Microsoft Developer Studio Project File - Name="libcommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libcommon - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libcommon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libcommon.mak" CFG="libcommon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libcommon - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libcommon - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libcommon - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\..\bin\Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\lib\Release"
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

!ELSEIF  "$(CFG)" == "libcommon - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\..\bin\Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\lib\Debug"
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

# Name "libcommon - Win32 Release"
# Name "libcommon - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\acmod_set.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\best_q.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\btree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\ck_seg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\ckd_alloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\cmd_ln.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\cmu6_lts_rules.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\cvt2triphone.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\dtree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\enum_subset.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\err.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\get_cpu_time.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\get_host_name.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\get_time.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\hash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\heap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\itree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\lexicon.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\lts.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\matrix.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\mk_phone_list.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\mk_phone_seq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\mk_sseq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\mk_trans_seq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\mk_ts2ci.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\mk_wordlist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\n_words.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\prefetch.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\prefix_upto.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\profile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\quest.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\remap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\state_seq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\timer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\ts2cb.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\two_class.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\vector.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\was_added.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\src\libs\libcommon\cmu6_lts_rules.h
# End Source File
# End Group
# End Target
# End Project
