/*
 Phonetisaurus.hpp

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
#ifndef PHONETISAURUS_H
#define PHONETISAURUS_H
#include <fst/fstlib.h>
#include "../g2p_train/FstPathFinder.hpp"
using namespace fst;
typedef PhiMatcher<SortedMatcher<Fst<StdArc> > > PM;

class Phonetisaurus {
    /*
       Load a G2P/P2G model and generate pronunciation/spelling
       hypotheses for input items.
     */
public:
    //Basics
    string eps;
    string se;
    string sb;
    string skip;
    string tie;
    set<string> skipSeqs;
    map<vector<string>,int> clusters;
    //FST stuff
    StdVectorFst *g2pmodel;
    StdVectorFst epsMapper;
    SymbolTable *isyms;
    SymbolTable *osyms;

    Phonetisaurus();

    Phonetisaurus(const char *_g2pmodel_file);

    StdVectorFst entryToFSA(vector<string> entry);

    StdVectorFst makeEpsMapper();

    vector<PathData>  phoneticize(vector<string> entry, int nbest,
                                    int beam = 500);

    bool printPaths(vector<PathData> paths, int nbest,
                    ofstream * hypfile, string correct = "", string word =
                        "", bool output_cost = true);

private:
    void loadClusters();
};

#endif                          // PHONETISAURUS_H //
