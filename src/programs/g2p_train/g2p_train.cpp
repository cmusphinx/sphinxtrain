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

/*
 * Port of phonetisaurus training procedure to C++ using openGrm
 * NGram language modeling toolkit instead of MITLM.
 *
 * for more details about phonetisaurus see
 * https://github.com/AdolfVonKleist/Phonetisaurus
 * http://www.openfst.org/twiki/bin/view/GRM/NGramLibrary
 */

#include <string>
#include <typeinfo>
#include <fst/symbol-table.h>
#include <fst/extensions/far/farscript.h>
#include <fst/script/fst-class.h>
#include <fst/script/determinize.h>
#include <fst/script/minimize.h>
#include <fst/extensions/far/far.h>
#include <fst/script/print.h>
#include <ngram/ngram-shrink.h>
#include <ngram/ngram-relentropy.h>
#include <ngram/ngram-seymore-shrink.h>
#include <ngram/ngram-count-prune.h>
#include <ngram/ngram-input.h>
#include <ngram/ngram-make.h>
#include <ngram/ngram-kneser-ney.h>
#include <ngram/ngram-absolute.h>
#include <ngram/ngram-katz.h>
#include <ngram/ngram-witten-bell.h>
#include <ngram/ngram-unsmoothed.h>
#include <sphinxbase/err.h>
#include <assert.h>
#include "include/PhonetisaurusRex.h"
#include "include/M2MFstAligner.h"
#include "include/LatticePruner.h"

#define arc_type "standard"
#define fst_type "vector"
#define far_type "default"
#define entry_type "line"
#define token_type "symbol"
#define generate_keys 0
#define unknown_symbol ""
#define keep_symbols true
#define initial_symbols true
#define allow_negative_labels false
#define key_prefix ""
#define key_suffix ""
#define backoff false
#define bins -1
#define backoff_label 0
#define norm_eps kNormEps
#define check_consistency false
#define discount_D -1
#define witten_bell_k 1
#define acceptor false
#define show_weight_one true
#define epsilon_as_backoff false
#define total_unigram_count -1.0
#define shrink_opt  2
namespace fst {

typedef LogWeightTpl<double> Log64Weight;
typedef ArcTpl<Log64Weight> Log64Arc;
typedef int StateId;
template <class Arc> struct ToLog64Mapper {
    typedef Arc FromArc;
    typedef Log64Arc ToArc;

    ToArc operator() (const FromArc & arc) const {
        return ToArc(arc.ilabel,
                     arc.olabel, arc.weight.Value(), arc.nextstate);
    } MapFinalAction FinalAction() const {
        return MAP_NO_SUPERFINAL;
    } MapSymbolsAction InputSymbolsAction() const {
        return MAP_COPY_SYMBOLS;
    } MapSymbolsAction OutputSymbolsAction() const {
        return MAP_COPY_SYMBOLS;
    } uint64 Properties(uint64 props) const {
        return props;
    }
};
}

using namespace std;
using namespace ngram;
using namespace fst;

string
convertInt(int number)
{
    stringstream ss;            //create a stringstream
    ss << number;               //add number to the stream
    return ss.str();            //return a string with the contents of the stream
}

void
addarcs(StateId state_id, StateId newstate, const SymbolTable * oldsyms,
        SymbolTable * isyms, SymbolTable * osyms, SymbolTable * ssyms,
        string eps, string s1s2_sep, StdMutableFst * fst,
        StdMutableFst * out)
{
    for (ArcIterator <StdFst> aiter(*fst, state_id); !aiter.Done();
            aiter.Next()) {
        StdArc arc = aiter.Value();
        string oldlabel = oldsyms->Find(arc.ilabel);
        // Make sure <eps> on its own maps to <eps>:<eps>
        if (oldlabel == eps) {
            // Ensure we aren't losing any output symbol (it's an
            // acceptor so this should not happen)
            assert(oldsyms->Find(arc.olabel) == eps);
            oldlabel = oldlabel.append("}");
            oldlabel = oldlabel.append(eps);
        }
        vector<string> tokens;
        tokens = tokenize_utf8_string(&oldlabel, &s1s2_sep);
        if (tokens[0] == "")
            tokens[0] = "_";
        if (tokens[1] == "")
            tokens[1] = "_";
        int64 ilabel = isyms->AddSymbol(tokens.at(0));
        int64 olabel = osyms->AddSymbol(tokens.at(1));

        int64 nextstate = ssyms->Find(convertInt(arc.nextstate));
        if (nextstate == -1) {
            nextstate = out->AddState();
            ssyms->AddSymbol(convertInt(arc.nextstate), nextstate);
        }
        out->AddArc(newstate,
                    StdArc(ilabel, olabel,
                           (arc.weight !=
                            TropicalWeight::Zero())? arc.
                           weight : TropicalWeight::One(), nextstate));
        //out->AddArc(newstate, StdArc(ilabel, olabel, arc.weight, nextstate));
    }
}

void
patch_labels(StdMutableFst *arpafst, SymbolTable* syms, int64 skip_id, bool input) {
    /*
      Patch all labels.  In some edge cases it is possible
      to end up grapheme subsequences: e.g. 'QU' where one or
      both tokens is only mapped to the multi-subsequence.  In thise
      case the independent 'Q' and/or 'U' token will never appear
      in isolation.
      This bit resolves this by:

      a.) finding and adding these missing subsequence tokens
      b.) adding backoff loops to the LM

    */
    string tie = "|";
    for (unsigned int i = skip_id + 1; i < syms->NumSymbols(); i++) {
        string sym = syms->Find(i);
        vector<string> parts  = tokenize_utf8_string(&sym, &tie);
        if (parts.size() > 1) {
            for (unsigned int j = 0; j < parts.size(); j++) {
                if (syms->Find(parts[j]) == -1) {
                    // Add the missing symbol
                    int k = syms->AddSymbol(parts[j]);
                    // Add a backoff loop mapped to the 'skip'
                    // FIXME: phonetisaurus hard-codes this as 1 but I
                    // believe that is wrong, it should maybe actually be
                    // ssyms->Find("<s>")?
                    int64 start_state = 1;
                    if (input == true)
                        arpafst->AddArc(start_state, StdArc(k, skip_id, 99, start_state));
                    else
                        arpafst->AddArc(start_state, StdArc(skip_id, k, 99, start_state));
                }
            }
        }
    }
}

void
relabel(StdMutableFst * fst, StdMutableFst * out, string prefix,
        string eps, string skip, string s1s2_sep, string seq_sep)
{
    /*
      Transform a statistical language model in ARPA format
      to an equivalent Weighted Finite-State Acceptor.
      This implementation adopts the Google format for the output
      WFSA.  This differs from previous implementations in several ways:

      Start-state and <s> arcs:
      * There are no explicit sentence-begin (<s>) arcs
      * There is a single <s> start-state.

      Final-state and </s> arcs:
      * There are no explicit sentence-end (</s>) arcs
      * There is no explicit </s> state
      * NGrams ending in </s> are designated as final
      states, and any probability is assigned
      to the final weight of said state.
    */
    namespace s = fst::script;
    using fst::ostream;
    using fst::SymbolTable;

    ArcSort(fst, StdILabelCompare());
    const SymbolTable *oldsyms = fst->InputSymbols();

    // generate new input, output and states SymbolTables
    SymbolTable *ssyms = new SymbolTable("ssyms");
    SymbolTable *isyms = new SymbolTable("isyms");
    SymbolTable *osyms = new SymbolTable("osyms");

    out->AddState();
    ssyms->AddSymbol("<s>");
    out->SetStart(0);

    string tie = "|";
    isyms->AddSymbol(eps);
    osyms->AddSymbol(eps);
    isyms->AddSymbol(tie);
    osyms->AddSymbol(tie);
    isyms->AddSymbol(skip);
    osyms->AddSymbol(skip);

    for (StateIterator<StdFst> siter(*fst); !siter.Done(); siter.Next()) {
        StateId state_id = siter.Value();

        int64 newstate;
        if (state_id == fst->Start()) {
            newstate = 0;
        }
        else {
            newstate = ssyms->Find(convertInt(state_id));
            if (newstate == -1) {
                newstate = out->AddState();
                ssyms->AddSymbol(convertInt(state_id), newstate);
            }
        }

        TropicalWeight weight = fst->Final(state_id);
        if (weight != TropicalWeight::Zero())
            out->SetFinal(newstate, weight);
        addarcs(state_id, newstate, oldsyms, isyms, osyms, ssyms, eps,
                s1s2_sep, fst, out);
    }

    patch_labels(out, isyms, isyms->Find(skip), true);
    patch_labels(out, osyms, osyms->Find(skip), false);
    out->SetInputSymbols(isyms);
    out->SetOutputSymbols(osyms);

    cout << "Writing text model to disk..." << endl;
    //Save syms tables
    isyms->WriteText(prefix + ".input.syms");
    osyms->WriteText(prefix + ".output.syms");

    string dest = prefix + ".fst.txt";
    fst::ofstream ostrm(dest.c_str());
    ostrm.precision(9);
    s::FstClass fstc(*out);
    s::PrintFst(fstc, ostrm, dest, isyms, osyms, NULL, acceptor, show_weight_one);
    ostrm.flush();
}

void
train_model(string eps, string s1s2_sep, string skip, int order,
            string smooth, string prefix, string seq_sep, string prune,
            double theta, string count_pattern)
{
    namespace s = fst::script;
    using fst::script::FstClass;
    using fst::script::MutableFstClass;
    using fst::script::VectorFstClass;
    using fst::script::WeightClass;

    // create symbols file (ngramsymbols)
    {
        cout << "Generating symbols..." << endl;
        NGramInput ingram(prefix + ".corpus.aligned", prefix + ".corpus.syms",
                          "", eps, unknown_symbol, "", "");
        // Magic!?
        ingram.ReadInput(0, 1);
    }

    // compile strings into a far archive (farcompilestrings)
    cout << "Compiling symbols into FAR archive..." << endl;
    fst::FarEntryType fet;
    fst::script::GetFarEntryType(entry_type, &fet);
    fst::FarTokenType ftt;
    fst::script::GetFarTokenType(token_type, &ftt);
    // Lovely inconsistent API you got there, OpenFST...
    fst::FarType fartype = fst::script::GetFarType(far_type);

    vector<string> in_fname;
    in_fname.push_back(prefix + ".corpus.aligned");
    fst::script::FarCompileStrings(in_fname,
                                   prefix + ".corpus.far", arc_type,
                                   fst_type, fartype,
                                   generate_keys, fet, ftt,
                                   prefix + ".corpus.syms",
                                   unknown_symbol, keep_symbols,
                                   initial_symbols, allow_negative_labels,
                                   key_prefix, key_suffix);

    // count n-grams (ngramcount)
    cout << "Counting n-grams..." << endl;
    NGramCounter<Log64Weight> ngram_counter(order, epsilon_as_backoff);
    FstReadOptions opts;
    // NO RAII FOR YOU! NO!
    FarReader<StdArc> *far_reader = FarReader<StdArc>::Open(prefix + ".corpus.far");
    int fstnumber = 1;
    const Fst<StdArc> *ifst = 0, *lfst = 0;
    while (!far_reader->Done()) {
        if (ifst)
            delete ifst;
        ifst = far_reader->GetFst()->Copy();

        if (!ifst) {
            E_FATAL("ngramcount: unable to read fst #%d\n", fstnumber);
            //exit(1);
        }

        bool counted = false;
        if (ifst->Properties(kString | kUnweighted, true)) {
            counted = ngram_counter.Count(*ifst);
        }
        else {
            VectorFst<Log64Arc> log_ifst;
            Map(*ifst, &log_ifst, ToLog64Mapper<StdArc> ());
            counted = ngram_counter.Count(&log_ifst);
        }
        if (!counted)
            cout << "ngramcount: fst #" << fstnumber << endl;

        if (ifst->InputSymbols() != 0) {        // retain for symbol table
            if (lfst)
                delete lfst;    // delete previously observed symbol table
            lfst = ifst;
            ifst = 0;
        }
        far_reader->Next();
        ++fstnumber;
    }
    delete far_reader;
    if (!lfst)
        E_FATAL("None of the input FSTs had a symbol table\n");
    VectorFst<StdArc> vfst;
    ngram_counter.GetFst(&vfst);
    ArcSort(&vfst, StdILabelCompare());
    vfst.SetInputSymbols(lfst->InputSymbols());
    vfst.SetOutputSymbols(lfst->InputSymbols());
    vfst.Write(prefix + ".corpus.cnts");

    // Make smoothed N-Grams (ngrammake)
    StdMutableFst *fst =
        StdMutableFst::Read(prefix + ".corpus.cnts", true);
    if (smooth != "no") {
        cout << "Smoothing model..." << endl;

        bool prefix_norm = 0;
        if (smooth == "presmoothed") {  // only for use with randgen counts
            prefix_norm = 1;
            smooth = "unsmoothed";      // normalizes only based on prefix count
        }
        if (smooth == "kneser_ney") {
            NGramKneserNey ngram(fst, backoff, backoff_label,
                                 norm_eps, check_consistency,
                                 discount_D, bins);
            ngram.MakeNGramModel();
            fst = ngram.GetMutableFst();
        }
        else if (smooth == "absolute") {
            NGramAbsolute ngram(fst, backoff, backoff_label,
                                norm_eps, check_consistency,
                                discount_D, bins);
            ngram.MakeNGramModel();
            fst = ngram.GetMutableFst();
        }
        else if (smooth == "katz") {
            // No idea why *this one* needs a template argument and
            // not the others!
            NGramKatz<StdArc> ngram(fst, backoff, backoff_label,
                            norm_eps, check_consistency, bins);
            ngram.MakeNGramModel();
            fst = ngram.GetMutableFst();
        }
        else if (smooth == "witten_bell") {
            NGramWittenBell ngram(fst, backoff, backoff_label,
                                  norm_eps, check_consistency,
                                  witten_bell_k);
            ngram.MakeNGramModel();
            fst = ngram.GetMutableFst();
        }
        else if (smooth == "unsmoothed") {
            NGramUnsmoothed ngram(fst, 1, prefix_norm, backoff_label,
                                  norm_eps, check_consistency);
            ngram.MakeNGramModel();
            fst = ngram.GetMutableFst();
        }
        else {
            E_FATAL("Bad smoothing method: %s\n", smooth.c_str());
        }
    }
    fst->Write(prefix + ".smooth.mod");
    if (prune != "no") {
        cout << "Pruning model..." << endl;
        if (prune == "count_prune") {
            NGramCountPrune ngramsh(fst, count_pattern,
                                    shrink_opt, total_unigram_count,
                                    backoff_label, norm_eps,
                                    check_consistency);
            ngramsh.ShrinkNGramModel();
        }
        else if (prune == "relative_entropy") {
            NGramRelEntropy ngramsh(fst, theta, shrink_opt,
                                    total_unigram_count, backoff_label,
                                    norm_eps, check_consistency);
            ngramsh.ShrinkNGramModel();
        }
        else if (prune == "seymore") {
            NGramSeymoreShrink ngramsh(fst, theta, shrink_opt,
                                       total_unigram_count, backoff_label,
                                       norm_eps, check_consistency);
            ngramsh.ShrinkNGramModel();
        }
        else {
            E_FATAL("Bad shrink method:  %s\n", prune.c_str());
        }
    }
    fst->Write(prefix + ".shrink.mod");

#if 0 // Don't do this, phonetisaurus doesn't do it
    cout << "Minimizing model..." << endl;
    MutableFstClass *minimized = new s::MutableFstClass(*fst);
    Minimize(minimized, 0, fst::kDelta);
    fst = minimized->GetMutableFst<StdArc>();
    fst->Write(prefix + ".min.mod");
#endif

    // Split input/output labels (phonetisaurus-arpa2wfst)
    cout << "Correcting final model..." << endl;
    StdMutableFst *out = new StdVectorFst();
    relabel(fst, out, prefix, eps, skip, s1s2_sep, seq_sep);

    cout << "Writing binary model to disk..." << endl;
    out->Write(prefix + ".fst");
}


void write_alignments (M2MFstAligner* aligner, std::ofstream &ofile,
               StdArc::Weight threshold, int nbest,
               bool fb, bool penalize) {
  /*
     Write the raw alignments to a file in text-based corpus format.

     NOTE: Although N-best and other pruning strategies are supported,
           the final format is that of a standard text corpus.  All relative
       token and pronunciation scores will be stripped.  In general
       this means that, unless you are very lucky with your combined
       pruning strategy the un-ranked N-best hypotheses will result in a
       lower-quality joint N-gram model.

       This approach is best used with simple 1-best.
  */

  //Build us a lattice pruner
  LatticePruner pruner (aligner->penalties, threshold, nbest, fb, penalize);

  VetoSet veto_set_;
  veto_set_.insert (0);
  for (unsigned int i = 0; i < aligner->fsas.size (); i++) {
    //Map to Tropical semiring
    VectorFst<StdArc>* tfst = new VectorFst<StdArc> ();
    Map (aligner->fsas.at (i), tfst, LogToStdMapper ());
    pruner.prune_fst (tfst);
    RmEpsilon (tfst);
    //Skip empty results.  This should only happen
    // in the following situations:
    //  1. seq1_del=false && len(seq1)<len(seq2)
    //  2. seq2_del=false && len(seq1)>len(seq2)
    //In both 1.and 2. the issue is that we need to
    // insert a 'skip' in order to guarantee at least
    // one valid alignment path through seq1*seq2, but
    // user params didn't allow us to.
    //Probably better to insert these where necessary
    // during initialization, regardless of user prefs.
    if (tfst->NumStates () > 0) {
      StdArc::Weight weight_threshold = 99;
      StdArc::StateId state_threshold = kNoStateId;
      AnyArcFilter<StdArc> arc_filter;
      vector<StdArc::Weight> distance;
      VectorFst<StdArc> ofst;

      AutoQueue<StdArc::StateId> state_queue (*tfst, &distance, arc_filter);
      IdentityPathFilter<StdArc> path_filter;

      ShortestPathOptions<StdArc, AutoQueue<StdArc::StateId>,
              AnyArcFilter<StdArc> >
    opts (&state_queue, arc_filter, nbest, false, false,
          kDelta, false, weight_threshold,
          state_threshold);
      ShortestPathSpecialized (*tfst, &ofst, &distance,
                   &path_filter, 10000, opts);
      for (size_t i = 0; i < path_filter.ordered_paths.size (); i++) {
    const vector<int>& path = path_filter.ordered_paths[i];
    for (size_t j = 0; j < path.size (); j++) {
      ofile << aligner->isyms->Find (path [j]);
      if (j < path.size () - 1)
        ofile << " ";
    }
    ofile << "\n";
      }
    }
    delete tfst;
  }

  return;
}


void
align(string input_file, string prefix, bool seq1_del, bool seq2_del,
      int seq1_max, int seq2_max, string seq_sep, string s1s2_sep,
      string eps, string skip, int iter)
{

    ifstream dict(input_file.c_str(), ifstream::in);
    string o = prefix + ".corpus.aligned";
    ofstream ofile(o.c_str(), ifstream::out);
    cout << "Loading..." << endl;
    M2MFstAligner fstaligner(seq1_del, seq2_del, seq1_max, seq2_max,
                             seq_sep, seq_sep, s1s2_sep, eps, skip,
                             // Fuck this stupid API
                             true, false, true, false);

    string sepnone = "";
    string septab = "\t";
    string sepspace = " ";
    string line;
    if (dict.is_open()) {
        while (dict.good()) {
            getline(dict, line);
            if (line.empty())
                continue;
            /* First try with tab */
            vector<string> tokens = tokenize_utf8_string(&line, &septab);
            if (tokens.size() != 2) {
                vector<string> tokens = tokenize_utf8_string(&line, &sepspace);
                if (tokens.size() < 2) {
                    cout << "Cannot parse line (must use tab or single space "
                         << "to separate word and phones):" << line << endl;
                    continue;
                }
                vector<string> seq1 = tokenize_utf8_string(&tokens.at(0), &sepnone);
                vector<string> seq2(tokens.begin() + 1, tokens.end());
                fstaligner.entry2alignfst(seq1, seq2);
            }
            else {
                vector<string> seq1 = tokenize_utf8_string(&tokens.at(0), &sepnone);
                vector<string> seq2 = tokenize_utf8_string(&tokens.at(1), &sepspace);
                fstaligner.entry2alignfst(seq1, seq2);
            }
        }
    }
    dict.close();

    cout << "Starting EM... " << endl;
    int i = 1;
    float change;
    change = fstaligner.maximization(false);
    for (i = 1; i <= iter; i++) {
        fstaligner.expectation();
        change = fstaligner.maximization(false);
        cout << "Iteration " << i << " Change: " << change << endl;
    }
    fstaligner.expectation();
    change = fstaligner.maximization(true);
    cout << "Last iteration: " << change << endl;

    cout << "Generating best alignments..." << endl;
    write_alignments(&fstaligner, ofile, -99.0, 1, false, true);
    ofile.flush();
    ofile.close();
}

void
split(string input_file, string prefix, int ratio)
{
    ifstream dict(input_file.c_str(), ifstream::in);

    string traindict = prefix + ".train";
    string testdict = prefix + ".test";
    ofstream trainfile(traindict.c_str(), ifstream::out);
    ofstream testfile(testdict.c_str(), ifstream::out);

    cout << "Splitting...\n" << endl;

    string line;
    int lineNum = 1;
    if (dict.is_open()) {
        while (dict.good()) {
            getline(dict, line);
            if (lineNum % ratio == 0) {
                testfile << line << endl;
            }
            else {
                trainfile << line << endl;
            }
            lineNum++;
        }
    }
    dict.close();
    trainfile.flush();
    testfile.flush();
    trainfile.close();
    testfile.close();
}
