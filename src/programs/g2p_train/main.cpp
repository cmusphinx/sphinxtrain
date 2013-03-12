/* ====================================================================
 * Copyright (c) 1995-2012 Carnegie Mellon University.  All rights
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

#include <iostream>
#include <vector>
#include <string>
#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>
#include "g2p_train.hpp"

using namespace std;

int
main(int argc, char *argv[])
{
    const char helpstr[] = "Train grapheme to phoneme (g2p) model for the dictionary";

    static arg_t defn[] = {
        {"-help", ARG_BOOLEAN, "no", "Shows the usage of the tool"},
        {   "-seq1_del", ARG_BOOLEAN, "no",
            "Allow deletions in sequence 1"
        },
        {   "-seq2_del", ARG_BOOLEAN, "no",
            "Allow deletions in sequence 2"
        },
        {   "-noalign", ARG_BOOLEAN, "no",
            "Do not align. Assume that the aligned corpus already exists"
        },
        {   "-seq1_max", ARG_INT32, "2",
            "Maximum subsequence length for sequence 1"
        },
        {   "-seq2_max", ARG_INT32, "2",
            "Maximum subsequence length for sequence 2"
        },
        {   "-skip", ARG_STRING, "_",
            "Skip/null symbol"
        },
        {   "-iter", ARG_INT32, "10",
            "Maximum number of iterations for EM"
        },
        {"-order", ARG_INT32, "6", "N-gram order"},
        {   "-prune", ARG_STRING, "no",
            "Pruning method. Available options are: 'no', 'count_prune', 'relative_entropy', 'seymore'"
        },
        {   "-theta", ARG_FLOATING, "0",
            "Theta value for 'relative_entropy' and 'seymore' pruning"
        },
        {   "-pattern", ARG_STRING, "",
            "Count cuttoffs for the various n-gram orders for 'count_prune' pruning"
        },
        {   "-smooth", ARG_STRING, "kneser_ney",
            "Smoothing method. Available options are: 'presmoothed', 'unsmoothed', 'kneser_ney', 'absolute', 'katz', 'witten_bell'"
        },
        {"-ifile", REQARG_STRING, "", "The input dictionary file."},
        {   "-gen_testset", ARG_BOOLEAN, "yes",
            "Whether or not a testset (1 in every 10 words) will be left out from the input dictionary, for model evaluation"
        },
        {   "-prefix", ARG_STRING, "model",
            "Prefix for saving the generated model and other files"
        },
        {   "-ratio", ARG_INT32, "10",
            "If a testset is generated, 1 word in every RATIO words will be left out from the input dictionary and inserted to the test set for model evaluation"
        },
        {NULL, 0, NULL, NULL}
    };


    cmd_ln_parse(defn, argc, argv, TRUE);

    if (cmd_ln_int32("-help")) {
        printf("%s\n\n", helpstr);
    }

    bool seq1_del = cmd_ln_boolean("-seq1_del");
    bool seq2_del = cmd_ln_boolean("-seq2_del");
    bool noalign = cmd_ln_boolean("-noalign");
    bool gen_testset = cmd_ln_boolean("-gen_testset");
    int seq1_max = cmd_ln_int32("-seq1_max");
    int seq2_max = cmd_ln_int32("-seq1_max");
    string seq_sep = "|";
    string s1s2_sep = "}";
    string eps = "<eps>";
    string skip = "_";
    int iter = cmd_ln_int32("-iter");
    int ratio = cmd_ln_int32("-ratio");
    int order = cmd_ln_int32("-order");
    string smooth = cmd_ln_str("-smooth");
    string prune = cmd_ln_str("-prune");
    double theta = cmd_ln_float32("-theta");
    string count_pattern = cmd_ln_str("-pattern");
    string ifile = cmd_ln_str("-ifile");
    string prefix = cmd_ln_str("-prefix");

    string input_file;

    if ((ifile.empty()) && !noalign) {
        E_FATAL("Input file not provided\n");
    }
    else {
        input_file = ifile;
    }
    if (prefix.empty()) {
        E_FATAL("Output file not provided\n");
    }
    if (gen_testset && !ifile.empty()) {
        cout << "Splitting dictionary: " << input_file <<
             " into training and test set" << endl;
        split(input_file, prefix, ratio);
        input_file = prefix + ".train";
    }

    if (!noalign) {
        cout << "Using dictionary: " << input_file << endl;
        align(input_file, prefix, seq1_del, seq2_del, seq1_max,
              seq2_max, seq_sep, s1s2_sep,
              eps, skip, iter);
    }

    train_model(eps, s1s2_sep, skip, order, smooth, prefix, seq_sep, prune,
                theta, count_pattern);

    return 0;
}
