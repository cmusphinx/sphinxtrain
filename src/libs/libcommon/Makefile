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
# 4. Products derived from this software may not be called "Sphinx"
#    nor may "Sphinx" appear in their names without prior written
#    permission of Carnegie Mellon University. To obtain permission,
#    contact sphinx@cs.cmu.edu.
#
# 5. Redistributions of any form whatsoever must retain the following
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
#  C libraries libcommon
#
# ====================================================================
TOP=../../..
DIRNAME=src/libs/libcommon
BUILD_DIRS = 
ALL_DIRS= $(BUILD_DIRS)
SRCS = \
  acmod_set.c \
  best_q.c \
  btree.c \
  ck_seg.c \
  ckd_alloc.c \
  cmd_ln.c \
  cvt2triphone.c \
  dtree.c \
  enum_subset.c \
  err.c \
  get_cpu_time.c \
  get_host_name.c \
  get_time.c \
  hash.c \
  heap.c \
  itree.c \
  lexicon.c \
  matrix.c \
  mk_phone_list.c \
  mk_phone_seq.c \
  mk_sseq.c \
  mk_trans_seq.c \
  mk_ts2ci.c \
  mk_wordlist.c \
  n_words.c \
  prefetch.c \
  prefix_upto.c \
  profile.c \
  quest.c \
  remap.c \
  state_seq.c \
  timer.c \
  ts2cb.c \
  two_class.c \
  vector.c \
  was_added.c
H =

FILES = Makefile $(SRCS) $(H)
LIBNAME= common

ALL = .build_lib

include $(TOP)/config/common_make_rules
