/*
 * FstPathFinder.hpp

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
  ----------------
    Original author: Chris Taylor

    OpenFst forum post title: "Natural code for printing all strings accepted by an FST?"
    OpenFst forum post link: http://openfst.cs.nyu.edu/twiki/bin/view/Forum/FstForum#Natural_code_for_printing_all_st

  ----------------

    2011-04-07: Modified by Josef Novak

    Modified to build a 'paths' object to store the individual paths
    and associated weights, rather than just print them out from
    inside the class.  Useful if you want to return the paths for further
    processing.
*
*/
#ifndef __FSTPATHFINDER__
#define __FSTPATHFINDER__

#include <fst/fstlib.h>

using namespace fst;

struct PathData {
    vector<string> path;
    float pathcost;
};

class FstPathFinder {

public:

    vector<PathData> paths;

    set<string> skipSeqs;

    set<vector <string> > uniqueStrings;

    SymbolTable *isyms;

    FstPathFinder();

    FstPathFinder(set<string> skipset);

    void findAllStrings(StdVectorFst & fst);

private:

    void addOrDiscardPath(PathData pdata);

    void findAllStringsHelper(StdVectorFst & fst,
                              int state,
                              vector<string> &str, TropicalWeight cost);

};                              // end class

#endif
