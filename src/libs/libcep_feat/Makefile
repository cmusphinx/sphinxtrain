# ====================================================================
# Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
#    endorse or promote products derived from this software without
#    prior written permission. To obtain permission, contact 
#    sphinx@cs.cmu.edu.
#
# 4. Redistributions of any form whatsoever must retain the following
#    acknowledgment:
#    "This product includes software developed by Carnegie
#    Mellon University (http://www.speech.cs.cmu.edu/)."
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ====================================================================
#
#  C libraries libcep_feat
#
# ====================================================================
TOP=../../..
DIRNAME=src/libs/libcep_feat
BUILD_DIRS = 
ALL_DIRS= $(BUILD_DIRS)
SRCS = \
  agc.c \
  agc_emax.c \
  agc_max.c \
  cep_frame.c \
  cmn.c \
  dcep_frame.c \
  ddcep_frame.c \
  del_sil_seg.c \
  feat.c \
  live_norm.c \
  norm.c \
  r_agc_noise.c \
  s2_cep.c \
  s2_dcep.c \
  s2_ddcep.c \
  s2_feat.c \
  silcomp.c \
  v1_feat.c \
  v2_feat.c \
  v3_feat.c \
  v4_feat.c \
  v5_feat.c \
  v6_feat.c \
  v7_feat.c \
  v8_feat.c
H = \
  agc_emax.h \
  agc_max.h \
  cep_frame.h \
  cep_manip.h \
  dcep_frame.h \
  ddcep_frame.h \
  del_sil_seg.h \
  feat_config.h \
  live_norm.h \
  norm.h \
  r_agc_noise.h \
  s2_cep.h \
  s2_dcep.h \
  s2_ddcep.h \
  s2_feat.h \
  v1_feat.h \
  v2_feat.h \
  v3_feat.h \
  v4_feat.h \
  v5_feat.h \
  v6_feat.h \
  v7_feat.h \
  v8_feat.h

FILES = Makefile $(SRCS) $(H)
LIBNAME= cep_feat

ALL = .build_lib

include $(TOP)/config/common_make_rules
