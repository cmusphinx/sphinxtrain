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

void PrintPathData (const vector<PathData>& results, string FLAGS_word,
		    const SymbolTable* osyms, bool print_scores,
		    bool nlog_probs) {
  for (int i = 0; i < results.size (); i++) {
    cout << FLAGS_word << "\t";
    if (print_scores == true) {
      if (nlog_probs == true) 
	cout << results [i].PathWeight << "\t";
      else
	cout << std::setprecision (3) << exp (-results [i].PathWeight) << "\t";
    }
    
    for (int j = 0; j < results [i].Uniques.size (); j++) {
      cout << osyms->Find (results [i].Uniques [j]);
      if (j < results [i].Uniques.size () - 1)
	cout << " ";
    }
    cout << endl;
  }    
}

void EvaluateWordlist (PhonetisaurusScript& decoder, vector<string> corpus,
		       int FLAGS_beam, int FLAGS_nbest, bool FLAGS_reverse,
		       string FLAGS_skip, double FLAGS_thresh, string FLAGS_gsep,
		       bool FLAGS_write_fsts, bool FLAGS_print_scores,
		       bool FLAGS_accumulate, double FLAGS_pmass,
		       bool FLAGS_nlog_probs) {
  for (int i = 0; i < corpus.size (); i++) {
    vector<PathData> results = decoder.Phoneticize (corpus [i], FLAGS_nbest,
						    FLAGS_beam, FLAGS_thresh,
						    FLAGS_write_fsts,
						    FLAGS_accumulate, FLAGS_pmass);
    PrintPathData (results, corpus [i],
		   decoder.osyms_,
		   FLAGS_print_scores,
		   FLAGS_nlog_probs);
  }
}
        

extern "C"
void
phoneticizeTestSet(const char *g2pmodel_file, const char *output,
                   const char *testset_file, int nbest, const char *sep,
                   int beam, int output_words, int output_cost)
{
    Phonetisaurus phonetisaurus(g2pmodel_file);

    ifstream test_fp;
    test_fp.open(testset_file.c_str());
    string line;

    if (test_fp.is_open()) {
        ofstream hypfile;
        hypfile.open(output);
        while (test_fp.good()) {
            getline(test_fp, line);
            if (line.compare("") == 0)
                continue;

            char *tmpstring = (char *) line.c_str();
            char *p = strtok(tmpstring, "\t");
            string word;
            string pron;

            int i = 0;
            while (p) {
                if (i == 0)
                    word = p;
                else
                    pron = p;
                i++;
                p = strtok(NULL, "\t");
            }

            vector <string> entry = tokenize_entry(&word, &sep,
                                                   phonetisaurus.isyms);
            vector<PathData> paths =
                phonetisaurus.phoneticize(entry, nbest, beam);
            int nbest_new = nbest;
            if (output_words == 0) {
                while (phonetisaurus.
                        printPaths(paths, nbest_new, &hypfile, output,
                                   pron) == true
                        && nbest_new <= paths.size()) {
                    nbest_new++;
                    paths =
                        phonetisaurus.phoneticize(entry, nbest_new, beam);
                }
            }
            else {
                while (phonetisaurus.
                        printPaths(paths, nbest_new, &hypfile, pron, word,
                                   output_cost) == true
                        && nbest_new <= paths.size()) {
                    nbest_new++;
                    paths =
                        phonetisaurus.phoneticize(entry, nbest_new, beam);
                }
            }
        }
        test_fp.close();
        hypfile.flush();
        hypfile.close();
    }
    else {
        cout << "Problem opening test file..." << endl;
    }

    return;
    vector<string> corpus;
    LoadWordList (testset_file, &corpus);
    
    PhonetisaurusScript decoder (g2pmodel_file);
    EvaluateWordlist (
        decoder, corpus, beam, nbest, false,
        "_", 99.0, "", false,
        true, false, 0.0,
        true
        );
}

extern "C"
void
phoneticizeWord(const char *g2pmodel_file, const char *output,
                const char *testword, int nbest, const char *sep, int beam,
                int output_words)
{
    PhonetisaurusScript phonetisaurus(g2pmodel_file);
    vector<PathData> paths = phonetisaurus.Phoneticize(
        testword, nbest, beam, 99.0,
        false, false, 0.0
        );
    ofstream hypfile;
    hypfile.open(output);

    if (output_words == 0) {
        while (phonetisaurus.printPaths(paths, nbest, &hypfile) == true
                && nbest <= paths.size()) {
            nbest++;
            paths = phonetisaurus.phoneticize(entry, nbest, beam);
        }
    }
    else {
        while (phonetisaurus.
                printPaths(paths, nbest, &hypfile, "", testword)
                == true && nbest <= paths.size()) {
            nbest++;
            paths = phonetisaurus.phoneticize(entry, nbest, beam);
        }
    }
    hypfile.flush();
    hypfile.close();

    return;
    PrintPathData (results, testword,
                   decoder.osyms_,
                   true,
                   true);
}
