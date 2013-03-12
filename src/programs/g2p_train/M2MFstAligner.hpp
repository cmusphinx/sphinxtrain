#ifndef M2MFSTALIGNER_H
#define M2MFSTALIGNER_H
/*
 M2MFstAligner.hpp

 Copyright (c) [2012-], Josef Robert Novak
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
  modification, are permitted #provided that the following conditions
  are met:

* Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above
    copyright notice, this list of #conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <fst/fstlib.h>
#include <vector>
#include "FstPathFinder.hpp"
using namespace std;

namespace fst {
class M2MFstAligner {
    /*
       Read in pairs of sequences of the form SEQ1 and SEQ2 and
       transform them into an FST that encodes all possible
       alignments between the symbols in the two sequences.
       Note that this may include a combination of multi-symbol
       subsequences depending on user specifications.

       This is achieved by simply generating the entire alignment
       graph during a single nested loop through the two input
       sequences that are to be aligned.

       The user may optionally specify whether to allow deletions
       for SEQ1 or SEQ2, as well as a maximum subsequence length
       for each sequence.
     */
public:
    //Basics declarations
    bool seq1_del;
    bool seq2_del;
    int seq1_max;
    int seq2_max;
    string seq1_sep;
    string seq2_sep;
    string s1s2_sep;
    string eps;
    string skip;
    bool penalize;
    vector<LogWeight> alpha, beta;
    //This will be used during decoding to clean the paths
    set<string> skipSeqs;
    //OpenFst stuff
    //These will be overwritten after each FST construction
    vector<VectorFst<LogArc> > fsas;

    //This will be maintained for the life of object
    //These symbol tables will be maintained entire life of
    // the object.  This will ensure that any resulting 'corpus'
    // shares the same symbol tables.
    SymbolTable *isyms;
    map<LogArc::Label,LogWeight> alignment_model;
    map<LogArc::Label,LogWeight> prev_alignment_model;
    LogWeight total;
    LogWeight prevTotal;

    //Constructors
    M2MFstAligner();
    M2MFstAligner(bool _seq1_del, bool _seq2_del, int _seq1_max,
                  int _seq2_max, string _seq1_sep, string _seq2_sep,
                  string _s1s2_sep, string _eps, string _skip,
                  bool _penalize);
    M2MFstAligner(string _model_file);

    //Write an aligner model to disk.  Critical info is stored in the
    // the symbol table so that it can be restored when the model is loaded.
    void write_model(string _model_name);
    //Transform a sequence pair into an equivalent multiple-to-multiple FST,
    // encoding all possible alignments between the two sequences
    void Sequences2FST(VectorFst<LogArc> *fst,
                       vector<string> *seq1,
                       vector<string> *seq2);
    void Sequences2FSTNoInit(VectorFst<LogArc> *fst,
                             vector<string> *seq1,
                             vector<string> *seq2);
    //Initialize all of the training data
    void entry2alignfst(vector<string> seq1,
                        vector<string> seq2);
    vector<PathData> entry2alignfstnoinit(vector<string> seq1,
            vector<string> seq2,
            int nbest,
            string lattice = "");
    vector<PathData> write_alignment_wrapper(int i, int nbest);
    //The expectation routine
    void expectation();
    //The maximization routine.  Returns the change since the last iteration
    float maximization(bool lastiter);
    //Print out the EM-optimized alignment for the training data
    vector<PathData> write_alignment(const VectorFst<LogArc>
                                        &ifst, int nbest);
    //Write out the union of the weighted alignment lattices for the training corpus
    void write_lattice(string lattice);
    //Convenience function to output all the alignments
    void write_all_alignments(int nbest);
    //max routine
    int get_max_length(string joint_label);
    int num_fsas();

};
}
#endif                          // M2MFSTALIGNER_H //
