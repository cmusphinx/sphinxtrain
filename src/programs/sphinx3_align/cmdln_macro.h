/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * cmdln_macro.h -- Corpus-file related misc functions.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.3  2006/02/22 19:37:51  arthchan2003
 * Removed command-line remove_zero_var_gau.  See the comments in revision 1.20 of cont_mgau.c
 *
 * Revision 1.2  2006/02/22 18:46:43  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: Add a macro files that
 * unified the command line of all the executables in Sphinx 3.6.
 * Affected executable includes decode, decode_anytopo, livepretend,
 * align, allphone, dag, astar and conf.  This will be commmented
 * individually in each executable.
 *
 * Revision 1.1.2.14  2006/01/16 19:50:19  arthchan2003
 * 1, Changed -ltsoov to -lts_mismatch, 2, Added an option to unscale the hypothesis segment scores. , 3, Added an option to dump the best senone scores.
 *
 * Revision 1.1.2.13  2005/11/17 06:11:15  arthchan2003
 * 1, Added -hypsegfmt which allows output of hyp(match)seg to s3, s2 and ctm segment format. 2, Added -outlatfmt, which allows conversion of sphinx 3 lattice to IBM lattice. 3, -latcompress, create an uncompressed(in terms of no. of links) version of dag.
 *
 * Revision 1.1.2.12  2005/10/07 18:55:10  arthchan2003
 * Fixed the display problem.
 *
 * Revision 1.1.2.11  2005/09/26 02:32:34  arthchan2003
 * (Change for comments) Also set agc default to none instead of max. The reason is that all 8 tests we have in the performance do not use -agc max. Also in practice, AGC usually hurt the performance.
 *
 * Revision 1.1.2.10  2005/09/26 02:21:57  arthchan2003
 * Changed the option -s3hmmdir to -hmm. This seems to be more generic and cause fewer restraints in future.
 *
 * Revision 1.1.2.9  2005/09/25 19:04:23  arthchan2003
 * Added macros for 1, tie routines which used vq_gen, 2, add support for using LTS rules to generate OOV. 3, enable and disable composite triphone support.
 *
 * Revision 1.1.2.8  2005/09/18 01:14:58  arthchan2003
 * Tie Viterbi history, DAG, debugging command line together.  Inspect all possible commands and specify whether they are mode-specific.
 *
 * Revision 1.1.2.7  2005/09/07 23:27:21  arthchan2003
 * Added an option in gmm_command_line_macro() to allow multiple behavior of Gaussian flooring.
 *
 * Revision 1.1.2.6  2005/08/03 19:58:11  arthchan2003
 * Change -topn from fast_GMM macro to acoustic model command line
 *
 * Revision 1.1.2.5  2005/08/03 18:53:59  dhdfu
 * Add -topn for SCHMM models
 *
 * Revision 1.1.2.4  2005/08/02 21:08:32  arthchan2003
 * 1, Changed -mean, -var, -tmat, -mixw -mdef to make them not required arguments. 2, Added -s3hmmdir so that user can just specified a directory name, in which all components of a set of HMM could be found.
 *
 * Revision 1.1.2.3  2005/07/24 01:43:59  arthchan2003
 * Temporarily not support -fsgctlfn
 *
 * Revision 1.1.2.2  2005/07/20 19:43:44  arthchan2003
 * Add command line arguments for fsg routines.
 *
 * Revision 1.1.2.1  2005/07/18 18:56:20  arthchan2003
 * A centralized macro definition file that contains command-line information that shared by different applications.
 *
 *
 *
 */

#ifndef _CMDLN_MACRO_H_
#define _CMDLN_MACRO_H_

/* Note by ARCHAN at 20050717 
   
The following are macros that used by individual modules and
routines. Since we want in one application, there is only one
command-line arguments. (The concept of singleton in Pattern).  If
we used variables, then every application that call this file and
its implementation will need got every command-line from every
application. That is not very nice. 

Using macro will temporarily solve this problem and I hope this
will only last for less than one minor version time.  A better way,
as what we have already done in Sphinx 4, is to ask the libraries
to register its command-line argument as well as configuration
arguments.  This will give user a better time of configuration and
us a better architecture. 

*/

#define vq_cluster_command_line_macro() \
    { "-stdev", \
      ARG_BOOLEAN, \
      "no", \
      "Use std.dev. (rather than var) in computing vector distances during clustering" }, \
    { "-eps", \
      ARG_FLOAT64, \
      "0.0001", \
      "Stopping criterion: stop iterations if relative decrease in sq(error) < eps" }, \
    { "-iter", \
      ARG_INT32, \
      "100", \
      "Max no. of k-means iterations for clustering" }

#define gmm_command_line_macro() \
    { "-mean",\
      ARG_STRING,\
      NULL,\
      "Mixture gaussian means input file" },\
    { "-var",\
      ARG_STRING,\
      NULL,\
      "Mixture gaussian variances input file" },\
    { "-varfloor",\
      ARG_FLOAT32,\
      "0.0001",\
      "Mixture gaussian variance floor (applied to data from -var file)" },\
    { "-mixw",\
      ARG_STRING,\
      NULL,\
      "Senone mixture weights input file" },\
    { "-mixwfloor",\
      ARG_FLOAT32,\
      "0.0000001",\
      "Senone mixture weights floor (applied to data from -mixw file)" }, \
{ "-sendump",                                                           \
      ARG_STRING,                                                               \
      NULL,                                                                     \
      "(S2 GMM computation only) Senone dump (compressed mixture weights) input file" }

#define acoustic_model_command_line_macro() \
    gmm_command_line_macro(), \
    { "-hmm", \
      ARG_STRING, \
      NULL, \
      "Directory for specifying Sphinx 3's hmm, the following files are assummed to be present, mdef, mean, var, mixw, tmat. If -mdef, -mean, -var, -mixw or -tmat are specified, they will override this command. "}, \
    { "-featparams",                                                        \
      ARG_STRING,                                                               \
      NULL,                                                                     \
      "File containing feature extraction parameters."},                        \
    { "-mdef", \
      ARG_STRING,\
      NULL,\
      "Model definition input file" },\
    { "-tmat",\
      ARG_STRING,\
      NULL,\
      "HMM state transition matrix input file" },\
    { "-tmatfloor",\
      ARG_FLOAT32,\
      "0.0001",\
      "HMM state transition probability floor (applied to -tmat file)" },\
    { "-senmgau",\
      ARG_STRING,\
      ".cont.",\
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" }, \
    { "-topn", \
      ARG_INT32, \
      "4", \
      "(S3.0 GMM Computation only) No. of top scoring densities computed in each mixture gaussian codebook (semi-continuous models only)" }, \
{ "-topn_beam",                                                                 \
      ARG_STRING,                                                               \
      "0",                                                                     \
      "(S2 GMM Computation only) Beam width used to determine top-N Gaussians (or a list, per-feature)" }

#define language_model_command_line_macro() \
    { "-lm", \
      ARG_STRING, \
      NULL, \
      "Word trigram language model input file" }, \
    { "-lmctlfn", \
      ARG_STRING, \
      NULL, \
      "Specify a set of language model\n"}, \
    { "-lmdumpdir", \
      ARG_STRING, \
      NULL, \
      "The directory for dumping the DMP file. "}, \
    { "-lmname", \
      ARG_STRING, \
      NULL, \
      "Name of language model in -lmctlfn to use for all utterances" }

#if 0
          /* Comment out because I don't know what is the meaning of Sphinx 2 fsg control file means. */
    { "-fsgctlfn",
      ARG_STRING,
      NULL,
      "A finite state grammar control file" },
#endif

#define finite_state_grammar_command_line_macro()  \
    { "-fsg", \
      ARG_STRING, \
      NULL, \
      "(FSG Mode (Mode 2) only) Finite state grammar"}, \
    { "-fsgusealtpron", \
      ARG_BOOLEAN, \
      "yes", \
      "(FSG Mode (Mode 2) only) Use alternative pronunciations for FSG"}, \
    { "-fsgusefiller", \
      ARG_BOOLEAN, \
      "yes", \
      "(FSG Mode (Mode 2) only) Insert filler words at each state."}


#define log_table_command_line_macro() \
    { "-logbase", \
      ARG_FLOAT32, \
      "1.0003", \
      "Base in which all log-likelihoods calculated" }, \
    { "-log3table", \
      ARG_BOOLEAN, \
      "yes", \
      "Determines whether to use the logs3 table or to compute the values at run time."}

#define phoneme_lookahead_command_line_macro() \
    { "-pheurtype", \
      ARG_INT32, \
      "0", \
      "0 = bypass, 1= sum of max, 2 = sum of avg, 3 = sum of 1st senones only" }, \
    { "-pl_window", \
      ARG_INT32, \
      "1", \
      "Window size (actually window size-1) of phoneme look-ahead." }, \
    { "-pl_beam", \
      ARG_FLOAT64, \
      "1.0e-80", \
      "Beam for phoneme look-ahead. [1 (narrowest)..10000000(very wide)]" }

#define histogram_pruning_command_line_macro() \
    { "-maxwpf", \
      ARG_INT32, \
      "20", \
      "(Only used in Mode 4 and 5) Max no. of distinct word exits to maintain at each frame" }, \
    { "-maxhistpf", \
      ARG_INT32, \
      "100", \
      "(Only used in Mode 4 and 5) Max no. of histories to maintain at each frame" }, \
    { "-hmmhistbinsize", \
      ARG_INT32, \
      "5000", \
      "(Only used in Mode 4 and 5) Performance histogram: #frames vs #HMMs active; #HMMs/bin in this histogram" }, \
    { "-maxhmmpf", \
      ARG_INT32, \
      "20000", \
      "(Only used in Mode 4 and 5) Max no. of active HMMs to maintain at each frame; approx." }

#define dictionary_command_line_macro() \
    { "-dict", \
      ARG_STRING, \
      NULL, \
      "Main pronunciation dictionary (lexicon) input file" }, \
    { "-fdict", \
      ARG_STRING, \
      NULL, \
      "Silence and filler (noise) word pronunciation dictionary input file" }, \
    { "-mdef_fillers", \
      ARG_BOOLEAN, \
      "no", \
      "Automatically add filler words from the model definition file" }, \
    { "-lts_mismatch", \
      ARG_BOOLEAN, \
      "no", \
      "Use CMUDict letter-to-sound rules to generate pronunciations for LM words doesn't appear in the dictionary . Use it with care. It assumes that the phone set in the mdef and dict are the same as the LTS rule. "}

#define gaussian_selection_command_line_macro() \
    { "-gs", \
      ARG_STRING, \
      NULL, \
      "Gaussian Selection Mapping." }

#define fast_GMM_computation_command_line_macro() \
    { "-subvq", \
      ARG_STRING, \
      NULL, \
      "Sub-vector quantized form of acoustic model" }, \
    { "-subvqbeam", \
      ARG_FLOAT64, \
      "3.0e-3", \
      "Beam selecting best components within each mixture Gaussian [0(widest)..1(narrowest)]" }, \
    gaussian_selection_command_line_macro(), \
    { "-ds", \
      ARG_INT32, \
      "1", \
      "Ratio of Down-sampling the frame computation." }, \
    { "-cond_ds", \
      ARG_BOOLEAN, \
      "no", \
      "Conditional Down-sampling, override normal down sampling. require specify a gaussian selection map" }, \
    { "-dist_ds", \
      ARG_BOOLEAN, \
      "no", \
      "Distance-based Down-sampling, override normal down sampling." }, \
    { "-gs4gs", \
      ARG_BOOLEAN, \
      "yes", \
      "A flag that specified whether the input GS map will be used for Gaussian Selection. If it is disabled, the map will only provide information to other modules." }, \
    { "-svq4svq", \
      ARG_BOOLEAN, \
      "no", \
      "A flag that specified whether the input SVQ will be used as approximate scores of the Gaussians" }, \
    { "-ci_pbeam", \
      ARG_FLOAT64, \
      "1e-80", /*default is huge , so nearly every cd phone will be computed */ \
      "CI phone beam for CI-based GMM Selection. [0(widest) .. 1(narrowest)]"}, \
    { "-tighten_factor", /* Use in "Down-sampling", this will tightened the beam \
			  */ \
      ARG_FLOAT64, \
      "0.5", \
      "From 0 to 1, it tightens the beam width when the frame is dropped"}, \
    { "-maxcdsenpf", \
      ARG_INT32, \
      "100000", \
      "Max no. of distinct CD senone will be computed. " }, \
    { "-vqeval", \
      ARG_INT32, \
      "3", \
      "Number of subvectors to use for SubVQ-based frame evaluation (3 for all)"}, \
    { "-kdtree",\
      ARG_STRING,\
      NULL,\
      "kd-Tree file for Gaussian selection (for .s2semi models only)" }, \
    { "-kdmaxdepth",\
      ARG_INT32,\
      "0",\
      "Maximum depth of kd-Trees to use" }, \
    { "-kdmaxbbi",\
      ARG_INT32,\
      "-1",\
      "Maximum number of Gaussians per leaf node in kd-Trees" }

#if 0
    { "-feat",
      ARG_STRING,
      "s2_4x",
      "Feature stream:\n\t\t\t\ts2_4x: Sphinx-II type 4 streams, 12cep, 24dcep, 3pow, 12ddcep\n\t\t\t\ts3_1x39: Single stream, 12cep+12dcep+3pow+12ddcep\n\t\t\t\t1s_12c_12d_3p_12dd: Single stream, 12cep+12dcep+3pow+12ddcep\n\t\t\t\t1s_c: Single stream, given input vector only\n\t\t\t\t1s_c_d: Feature + Deltas only\n\t\t\t\t1s_c_dd: Feature + Double deltas only\n\t\t\t\t1s_c_d_dd: Feature + Deltas + Double deltas\n\t\t\t\t1s_c_wd_dd: Feature cep+windowed delcep+deldel \n\t\t\t1s_c_d_ld_dd: Feature + delta + longter delta + doubledelta" },

    { "-feat",	/* Captures the computation for converting input to feature vector */
      ARG_STRING,
      "1s_c_d_dd",
      "Feature stream: s2_4x / s3_1x39 / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
#endif


#define speaker_adaptation_command_line_macro() \
    { "-mllr", \
      ARG_STRING, \
      NULL, \
      "MLLR transfomation matrix to be applied to mixture gaussian means"}, \
    { "-cb2mllr", \
      ARG_STRING, \
      ".1cls.", \
      "Senone to MLLR transformation matrix mapping file (or .1cls.)" }


#define common_filler_properties_command_line_macro() \
    { "-fillpen", \
      ARG_STRING, \
      NULL, \
      "Filler word probabilities input file (used in place of -silpen and -noisepen)" }, \
    { "-silprob", \
      ARG_FLOAT32, \
      "0.1", \
      "Default silence word probability" }, \
    { "-fillprob", \
      ARG_FLOAT32, \
      "0.1", \
      "Default non-silence filler word probability" }, \
    { "-lw", \
      ARG_FLOAT32, \
      "9.5", \
      "Language weight" }, \
    { "-wip", \
      ARG_FLOAT32, \
      "0.7", \
      "Word insertion penalty" }, \
    { "-uw", \
      ARG_FLOAT32, \
      "0.7", \
      "Unigram weight" }

          /* Follow sphinx 3.0, this should be implemented in nearly
             every tool */
#define phone_insertion_penalty_command_line_macro() \
    { "-phonepen", \
      ARG_FLOAT32, \
      "1.0", \
      "(Mode 2 and 3 only) Word insertion penalty" }


#define common_s3x_beam_properties_command_line_macro() \
    { "-beam", \
      ARG_FLOAT64, \
      "1.0e-55", \
      "Beam selecting active HMMs (relative to best) in each frame [0(widest)..1(narrowest)]" }, \
    { "-pbeam", \
      ARG_FLOAT64, \
      "1.0e-50", \
      "Beam selecting HMMs transitioning to successors in each frame [0(widest)..1(narrowest)]" }, \
    { "-wbeam", \
      ARG_FLOAT64, \
      "1.0e-35", \
      "Beam selecting word-final HMMs exiting in each frame [0(widest)..1(narrowest)]" }, \
    { "-wend_beam", \
      ARG_FLOAT64, \
      "1.0e-80", \
      "Beam selecting word-final HMMs exiting in each frame [0(widest) .. 1(narrowest)]" }, \
    { "-ptranskip", \
      ARG_INT32, \
      "0", \
      "(Not used in Mode 3) Use wbeam for phone transitions every so many frames (if >= 1)" }

#define common_application_properties_command_line_macro()      \
    { "-logfn",                                                 \
      ARG_STRING,                                               \
      NULL,                                                     \
            "Log file (default stdout/stderr)" },               \
    { "-build_outdirs",                                         \
        ARG_BOOLEAN,                                            \
            "yes",                                              \
            "Create parent directories for output files" }

#define control_file_handling_command_line_macro() \
    { "-ctl", \
      ARG_STRING, \
      NULL, \
      "Control file listing utterances to be processed" }, \
    { "-ctloffset", \
      ARG_INT32, \
      "0", \
      "No. of utterances at the beginning of -ctl file to be skipped" }, \
    { "-ctlcount", \
      ARG_INT32, \
      "1000000000",	/* A big number to approximate the default: "until EOF" */ \
      "No. of utterances to be processed (after skipping -ctloffset entries)" }

#define hypothesis_file_handling_command_line_macro() \
    { "-hyp", \
      ARG_STRING, \
      NULL, \
      "Recognition result file, with only words" }, \
    { "-hypseg", \
      ARG_STRING, \
      NULL, \
      "Recognition result file, with word segmentations and scores" }

#define score_handling_command_line_macro() \
    { "-hypsegscore_unscale", \
      ARG_BOOLEAN, \
      "yes", \
      "When displaying the results, whether to unscale back the acoustic score with the best score in a frame"}

#define cepstral_input_handling_command_line_macro() \
    { "-cepdir", \
      ARG_STRING, \
      ".", \
      "Input cepstrum files directory (prefixed to filespecs in control file)" }, \
    { "-cepext", \
      ARG_STRING, \
      ".mfc", \
      "Input cepstrum files extension (prefixed to filespecs in control file)" },  \
    { "-adcin", \
      ARG_BOOLEAN, \
      "no", \
      "Input is waveform data rather than cepstra (-cepdir and -cepext are still used)" }, \
    { "-adchdr", \
      ARG_INT32, \
      "0", \
      "Number of bytes to skip at the beginning of a waveform file (44 for WAV, 1024 for Sphere)" }

#define output_lattice_handling_command_line_macro() \
    { "-outlatdir", \
      ARG_STRING, \
      NULL, \
      "Directory in which to dump word lattices" }, \
    { "-outlatfmt", \
      ARG_STRING, \
      "s3", \
      "Format in which to dump word lattices (either 's3' or 'htk')" }, \
    { "-latext", \
      ARG_STRING, \
      "lat.gz", \
      "Filename extension for lattice files (gzip compressed, by default - remove .gz for uncompressed)" }



#define history_table_command_line_macro() \
    { "-bptbldir", \
      ARG_STRING, \
      NULL, \
      "Directory in which to dump word Viterbi back pointer table (for debugging)" }, \
    { "-bptblsize", \
      ARG_INT32, \
      "32768", \
      "Number of BPtable entries to allocate initially (grown as necessary)" }

/* decode-specific, that includes mode 4 and mode 5'
   share between decode/livepretend/livedecode
*/
#define decode_specific_command_line_macro() \
    { "-mode", \
    ARG_STRING, \
    "fwdtree",\
    "Decoding mode, one of allphone, fsg, fwdflat, fwdtree."}, \
    { "-op_mode", \
      ARG_INT32, \
      "-1", \
      "Operation mode, for internal use only."}, \
    { "-hmmdump", \
      ARG_BOOLEAN, \
      "no", \
      "Whether to dump active HMM details to stderr (for debugging)" }, \
    { "-lextreedump", \
      ARG_INT32, \
      "0", \
      "Whether to dump the lextree structure to stderr (for debugging), 1 for Ravi's format, 2 for Dot format, Larger than 2 will be treated as Ravi's format" }, \
    { "-bghist", \
      ARG_BOOLEAN, \
      "no", \
      "Bigram-mode: If TRUE only one BP entry/frame; else one per LM state" }, \
    { "-treeugprob", \
      ARG_BOOLEAN, \
      "yes", \
      "If true, Use unigram probs in lextree" }

#define dag_handling_command_line_macro() \
    { "-min_endfr", \
      ARG_INT32, \
      "3", \
      "Nodes ignored during search if they persist for fewer than so many end frames" }, \
    { "-dagfudge", \
      ARG_INT32, \
      "2", \
      "(0..2); 1 or 2: add edge if endframe == startframe; 2: if start == end-1" }, \
    { "-maxedge", \
      ARG_INT32, \
      "2000000", \
      "Max DAG edges allowed in utterance; aborted if exceeded; controls memory usage" }, \
    { "-maxlmop", \
      ARG_INT32, \
      "100000000", \
      "Max LMops in utterance after which it is aborted; controls CPU use (see maxlpf)" }, \
    { "-maxlpf", \
      ARG_INT32, \
      "40000", \
      "Max LMops/frame after which utterance aborted; controls CPU use (see maxlmop)" }, \
    {"-latcompress", \
      ARG_BOOLEAN, \
      "yes", \
      "Whether lattice is compressed."}


#define second_stage_dag_handling_command_line_macro() \
    { "-bestpath", \
      ARG_BOOLEAN, \
      "no", \
      "Whether to run bestpath DAG search after forward Viterbi pass" }, \
    { "-bestpathlw", \
      ARG_FLOAT32, \
      NULL, \
      "Language weight for bestpath DAG search (default: same as -lw)" }, \
    {"-nbestdir", \
     ARG_STRING, \
     NULL, \
     "Input word-lattice directory with per-utt files for restricting words searched"}, \
    {"-nbestext", \
     ARG_STRING, \
     "nbest.gz", \
     "N-best filename extension (.gz or .Z extension for compression)"}, \
    {"-nbest", \
     ARG_INT32, \
     "200", \
     "Max. n-best hypotheses to generate per utterance"}, \
    {"-maxppath", \
     ARG_INT32, \
     "1000000", \
     "Max partial paths created after which utterance aborted; controls CPU/memory use"}, \
    {"-ppathdebug", \
     ARG_BOOLEAN, \
     "no", \
     "Generate debugging information for N-best search. "}

#define input_lattice_handling_command_line_macro() \
    { "-inlatdir", \
      ARG_STRING, \
      NULL, \
      "Input word-lattice directory with per-utt files for restricting words searched" }, \
    { "-inlatwin", \
      ARG_INT32, \
      "50", \
      "Input word-lattice words starting within +/- <this argument> of current frame considered during search" }

#define flat_fwd_debugging_command_line_macro() \
    { "-tracewhmm", \
      ARG_STRING, \
      NULL, \
      "(Mode 3 only) Word whose active HMMs are to be traced (for debugging/diagnosis/analysis)" }, \
    { "-hmmdumpef", \
      ARG_INT32, \
      "200000000", \
      "(Mode 3 only) Ending frame for dumping all active HMMs (for debugging/diagnosis/analysis)" }, \
    { "-hmmdumpsf", \
      ARG_INT32, \
      "200000000", \
      "(Mode 3 only) Starting frame for dumping all active HMMs (for debugging/diagnosis/analysis)" }, \
    { "-worddumpef", \
      ARG_INT32, \
      "200000000", \
      "(Mode 3 only) Ending frame for dumping all active words (for debugging/diagnosis/analysis)" }, \
    { "-worddumpsf", \
      ARG_INT32, \
      "200000000", \
      "(Mode 3 only) Starting frame for dumping all active words (for debugging/diagnosis/analysis)" }


#define search_specific_command_line_macro() \
    {"-backtrace", \
     ARG_BOOLEAN, \
      "yes", \
      "Whether detailed backtrace information (word segmentation/scores) shown in log" }, \
    { "-bestsenscrdir", \
      ARG_STRING, \
      NULL, \
      "When Best senone score directory." }


/* mode TST or mode 4*/
#define search_modeTST_specific_command_line_macro() \
    { "-Nlextree", \
      ARG_INT32, \
      "3", \
      "(Mode 4 only) No. of lextrees to be instantiated; entries into them staggered in time" }, \
    { "-epl", \
      ARG_INT32, \
      "3", \
      "(Mode 4 only) Entries Per Lextree; #successive entries into one lextree before lextree-entries shifted to the next" }

/* mode WST or mode 5*/
#define search_modeWST_specific_command_line_macro() \
    { "-Nstalextree", \
      ARG_INT32, \
      "25", \
      "(Mode 5 only) No. of lextrees to be instantiated statically; " }

#define partial_hypothesis_command_line_macro() \
    { "-maxhyplen", \
      ARG_INT32, \
      "1000", \
      "(Live-decoder only) Maximum number of words in a partial hypothesis (for block decoding)" }, \
    { "-phypdump", \
      ARG_BOOLEAN, \
      "yes", \
      "(Live-decoder only) dump parital hypothesis on the screen"}

#define control_lm_file_command_line_macro() \
    { "-ctl_lm", \
      ARG_STRING, \
      NULL, \
      "(Not used in mode 2 and 3) Control file that list the corresponding LMs" }

#define control_mllr_file_command_line_macro() \
    { "-ctl_mllr", \
      ARG_STRING, \
      NULL, \
      "Control file that list the corresponding MLLR matrix for an utterance"}

    /* the following functions are used for MMIE training
       lqin 2010-03 */
#define unigram_only_lm_command_line_macro() \
        { "-ugonly", \
          ARG_BOOLEAN, \
          "no", \
          "Only use Unigram language model in decoding"}

#define bigram_only_lm_command_line_macro() \
        { "-bgonly", \
          ARG_BOOLEAN, \
          "no", \
          "Only use Bigram language model in decoding"}
    /* end */

#define control_lm_mllr_file_command_line_macro() \
    control_lm_file_command_line_macro(), \
    control_mllr_file_command_line_macro()

#endif /* _CMDLN_MACRO_H_ */
