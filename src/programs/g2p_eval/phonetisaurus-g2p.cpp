/*
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

#include <iostream>
#include "Phonetisaurus.hpp"
#include "util.hpp"

using namespace fst;

void
phoneticizeWord(const char *g2pmodel_file, const char *output,
                string testword, int nbest, string sep, int beam = 500,
                int output_words = 0)
{

    Phonetisaurus phonetisaurus(g2pmodel_file);

    vector <string> entry =
        tokenize_entry(&testword, &sep, phonetisaurus.isyms);

    vector<PathData> paths =
        phonetisaurus.phoneticize(entry, nbest, beam);
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
}

void
phoneticizeTestSet(const char *g2pmodel_file, const char *output,
                   string testset_file, int nbest, string sep, int beam =
                       500, int output_words = 0, bool output_cost = true)
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
}
