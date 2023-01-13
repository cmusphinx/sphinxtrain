/*
 phonetisaurus-g2pfst.cc

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
*
*/
using namespace std;
#include <include/PhonetisaurusScript.h>
#include <include/util.h>
#include <iomanip>
#include <fst/fstlib.h>
using namespace fst;

#include "phonetisaurus-g2p.h"

typedef unordered_map<int, vector<PathData> > RMAP;

void PrintPathData(ofstream &output,
                   const vector<PathData>& results, string FLAGS_word,
                   const SymbolTable* osyms, bool print_scores,
                   bool nlog_probs, bool output_words) {
    for (int i = 0; i < results.size(); i++) {
        if (output_words)
            output << FLAGS_word << "\t";
        if (print_scores == true) {
            if (nlog_probs == true) 
                output << results[i].PathWeight << "\t";
            else
                output << std::setprecision (3) << exp(-results[i].PathWeight) << "\t";
        }
    
        for (int j = 0; j < results[i].Uniques.size(); j++) {
            output << osyms->Find(results[i].Uniques[j]);
            if (j < results[i].Uniques.size() - 1)
                output << " ";
        }
        output << endl;
    }    
}

extern "C"
void
phoneticizeTestSet(const char *g2pmodel_file, const char *output,
                   const char *testset_file, int nbest, const char *sep,
                   int beam, int output_words, int output_cost)
{
    PhonetisaurusScript decoder(g2pmodel_file, sep);
    
    vector<string> corpus;
    bool write_fsts = false;
    bool accumulate = false;
    double pmass = 99.0;
    LoadWordList(testset_file, &corpus);
    ofstream hypfile;
    hypfile.open(output);
    for (int i = 0; i < corpus.size(); i++) {
        vector<PathData> results = decoder.Phoneticize(corpus[i], nbest,
                                                       beam, 99.0,
                                                       write_fsts,
                                                       accumulate, pmass);
        PrintPathData(hypfile, results, corpus[i],
                      decoder.osyms_, output_cost, true, output_words);
    }
}

extern "C"
void
phoneticizeWord(const char *g2pmodel_file, const char *output,
                const char *testword, int nbest, const char *sep, int beam,
                int output_words, int output_cost)
{
    PhonetisaurusScript decoder(g2pmodel_file, sep);
    bool write_fsts = false;
    bool accumulate = false;
    vector<PathData> results = decoder.Phoneticize(testword, nbest,
                                                   beam, 99.0, write_fsts, accumulate, 0.0);
    ofstream hypfile;
    hypfile.open(output);

    PrintPathData(hypfile, results, testword,
                  decoder.osyms_, output_cost, true, output_words);
}
