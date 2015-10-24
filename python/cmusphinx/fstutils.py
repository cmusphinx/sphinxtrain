#!/usr/bin/env python

# Copyright (c) 2010  Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
FST utility functions
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision $"

import sys
import os
import tempfile
import openfst
import sphinxbase
import subprocess

class AutoFst(openfst.StdVectorFst):
    """
    FST class which automatically adds states, input and output symbols as required.

    This is meant to behave somewhat like the Dot language.
    """
    def __init__(self, isyms=None, osyms=None, ssyms=None):
        openfst.StdVectorFst.__init__(self)
        if isyms == None:
            isyms = openfst.SymbolTable("inputs")
            isyms.AddSymbol("&epsilon;")
        if osyms == None:
            osyms = openfst.SymbolTable("outputs")
            osyms.AddSymbol("&epsilon;")
        if ssyms == None:
            ssyms = openfst.SymbolTable("states")
            ssyms.AddSymbol("__START__")
        self.ssyms = ssyms
        self.SetInputSymbols(isyms)
        self.SetOutputSymbols(osyms)
        self.SetStart(self.AddState())
        
    def AddArc(self, src, isym, osym, weight, dest):
        if not isinstance(isym, int):
            isym = self.isyms.AddSymbol(isym)
        if not isinstance(osym, int):
            osym = self.osyms.AddSymbol(osym)
        if not isinstance(src, int):
            src = self.ssyms.AddSymbol(src)
        if not isinstance(dest, int):
            dest = self.ssyms.AddSymbol(dest)
        while src >= self.NumStates():
            self.AddState()
        while dest >= self.NumStates():
            self.AddState()
        openfst.StdVectorFst.AddArc(self, src, isym, osym, weight, dest)

    def Write(self, *args):
        openfst.StdVectorFst.SetInputSymbols(self, self.isyms)
        openfst.StdVectorFst.SetOutputSymbols(self, self.osyms)
        openfst.StdVectorFst.Write(self, *args)

    def SetFinal(self, state, weight=0):
        if not isinstance(state, int):
            state = self.ssyms.AddSymbol(state)
        openfst.StdVectorFst.SetFinal(self, state, weight)

    def SetInputSymbols(self, isyms):
        self.isyms = isyms
        openfst.StdVectorFst.SetInputSymbols(self, self.isyms)

    def SetOutputSymbols(self, osyms):
        self.osyms = osyms
        openfst.StdVectorFst.SetOutputSymbols(self, self.osyms)

def add_mgram_states(fst, symtab, lm, m, sidtab, bo_label=0):
    """
    Add states and arcs for all M-grams in the language model, where M<N.
    """
    for mg in lm.mgrams(m):
        wsym = symtab.Find(mg.words[m])
        if wsym == -1:
            continue # skip mgrams ending in OOV
        if m > 0 and mg.words[0] == '</s>':
            continue # skip >1-grams starting with </s>
        if m == 0:
            src = 0 # 1-grams start in backoff state
        elif tuple(mg.words[0:m]) not in sidtab:
            continue # this means it has an OOV
        else:
            src = sidtab[tuple(mg.words[0:m])]
        if mg.words[m] == '</s>':
            # only one final state is allowed
            final = True
            newstate = False
            if ('</s>',) in sidtab:
                dest = sidtab[('</s>',)]
            else:
                dest = fst.AddState()
                fst.SetFinal(dest, 0)
                sidtab[('</s>',)] = dest
                #print "Final state", dest
                #print "Entered state ID mapping (</s>,) =>", dest
        else:
            final = False
            newstate = True
            dest = fst.AddState()

        if mg.words[m] == '<s>':
            # <s> is a non-event
            if m == 0:
                # The destination state will be the initial state
                fst.SetStart(dest)
                #print "Initial state", dest
        else:
            fst.AddArc(src, openfst.StdArc(wsym, wsym, -mg.log_prob, dest))
            #print "Added %d-gram arc %d => %d %s/%.4f" % (m+1, src, dest,
            #mg.words[m], -mg.log_prob)

        if newstate:
            # Add a new state to the mapping if needed
            sidtab[tuple(mg.words)] = dest
            #print "Entered state ID mapping", tuple(mg.words), "=>", dest

        if not final:
            # Create a backoff arc to the suffix M-1-gram
            # Note taht if mg.log_bowt == 0 it's particularly important to do this!
            if m == 0:
                bo_state = 0 # backoff state
            elif tuple(mg.words[1:]) in sidtab:
                bo_state = sidtab[tuple(mg.words[1:])]
            else:
                continue # Not a 1-gram, no suffix M-gram
            fst.AddArc(dest, openfst.StdArc(bo_label, bo_label, -mg.log_bowt, bo_state))
            #print "Adding backoff arc %d => %d %.4f" % (dest, bo_state, -mg.log_bowt)

def add_ngram_arcs(fst, symtab, lm, n, sidtab):
    """
    Add states and arcs for all N-grams in the language model, where
    N=N (the order of the model, that is).
    """
    for ng in lm.mgrams(n-1):
        wsym = symtab.Find(ng.words[n-1])
        if wsym == -1: # OOV
            continue
        if ng.words[n-1] == '<s>': # non-event
            continue
        if '</s>' in ng.words[0:n-1]:
            continue
        for w in ng.words[:n-1]: # skip OOVs
            if symtab.Find(w) == -1:
                #print w, "not found"
                continue
        src = sidtab[tuple(ng.words[:n-1])]
        # Find longest suffix N-gram that exists
        spos = 1
        while tuple(ng.words[spos:]) not in sidtab:
            spos += 1
            if spos == n:
                raise RuntimeError, "Unable to find suffix N-gram for", ng.wids
        dest = sidtab[tuple(ng.words[spos:])]
        fst.AddArc(src, openfst.StdArc(wsym, wsym, -ng.log_prob, dest))
        #print "Adding %d-gram arc %d => %d %s/%.4f" % (n, src, dest, ng.words[n-1], -ng.log_prob)
            
def build_lmfst(lm, use_phi=False):
    """
    Build an FST recognizer from an N-gram backoff language model.
    """
    fst = openfst.StdVectorFst()
    symtab = openfst.SymbolTable("words")
    epsilon = symtab.AddSymbol("&epsilon;")
    if use_phi:
        phi = symtab.AddSymbol("&phi;")
        bo_label = phi
    else:
        bo_label = epsilon
    for ug in lm.mgrams(0):
        wsym = symtab.AddSymbol(ug.words[0])
    fst.SetInputSymbols(symtab)
    fst.SetOutputSymbols(symtab)
    # The algorithm goes like this:
    #
    # Create a backoff state
    # For M in 1 to N-1:
    #  For each M-gram w(1,M):
    #   Create a state q(1,M)
    #   Create an arc from state q(1,M-1) to q(1,M) with weight P(w(1,M))
    #   Create an arc from state q(1,M) to q(2,M) with weight bowt(w(1,M-1))
    # For each N-gram w(1,N):
    #   Create an arc from state q(1,N-1) to q(2,N) with weight P(w(1,N))

    # Table holding M-gram to state mappings
    sidtab = {}
    fst.AddState() # guaranteed to be zero (we hope)
    for m in range(lm.get_size() - 1):
        add_mgram_states(fst, symtab, lm, m, sidtab, bo_label)
    add_ngram_arcs(fst, symtab, lm, lm.get_size(), sidtab)

    # Connect and arc-sort the resulting FST
    openfst.Connect(fst)
    openfst.ArcSortInput(fst)
    return fst

class SphinxProbdef(object):
    """
    Probability definition file used for Sphinx class language models.
    """
    def __init__(self, infile=None):
        self.classes = {}
        if infile != None:
            self.read(infile)

    def read(self, infile):
        """
        Read probability definition from a file.
        """
        if not isinstance(infile, file):
            infile = file(infile)
        inclass = None
        for spam in infile:
            spam = spam.strip()
            if spam.startswith('#') or spam.startswith(';'):
                continue
            if spam == "":
                continue
            if inclass:
                parts = spam.split()
                if len(parts) == 2 \
                       and parts[0] == "END" and parts[1] == classname:
                    inclass = None
                else:
                    prob = 1.0
                    if len(parts) > 1:
                        prob = float(parts[1])
                    self.add_class_word(inclass, parts[0], prob)
            else:
                if spam.startswith('LMCLASS'):
                    foo, classname = spam.split()
                    self.add_class(classname)
                    inclass = classname

    def add_class(self, name):
        """
        Add a class to this probability definition.
        """
        self.classes[name] = {}

    def add_class_word(self, name, word, prob):
        """
        Add a word to a class in this probability definition.
        """
        self.classes[name][word] = prob
    
    def write(self, outfile):
        """
        Write out probability definition to a file.
        """
        if not isinstance(outfile, file):
            outfile = file(outfile)
        for c in self.classes:
            outfile.write("LMCLASS %s\n" % c)
            for word, prob in self.classes[c]:
                outfile.write("%s %g\n" % (word, prob))
            outfile.write("END %s\n" % c)
            outfile.write("\n")

    def normalize(self):
        """
        Normalize probabilities.
        """
        for c in self.classes:
            t = sum(self.classes[c].itervalues())
            if t != 0:
                for w in self.classes[c]:
                    self.classes[c][w] /= t

def build_classfst(probdef, isyms=None):
    """
    Build an FST from the classes in a Sphinx probability definition
    file.  This transducer maps words to classes, and can either be
    composed with the input, or pre-composed with the language model.
    In the latter case you can project the resulting transducer to its
    input to obtain an equivalent non-class-based model.
    """
    if not isinstance(probdef, SphinxProbdef):
        probdef = SphinxProbdef(probdef)
    fst = openfst.StdVectorFst()
    if isyms:
        symtab = isyms
    else:
        symtab = openfst.SymbolTable("words")
        symtab.AddSymbol("&epsilon;")
    st = fst.AddState()
    fst.SetStart(st)
    fst.SetFinal(st, 0)
    for word, label in symtab:
        if label == openfst.epsilon:
            continue
        fst.AddArc(st, label, label, 0, st)
    for c in probdef.classes:
        clabel = symtab.AddSymbol(c)
        for word, prob in probdef.classes[c].iteritems():
            wlabel = symtab.AddSymbol(word)
            fst.AddArc(st, wlabel, clabel, -math.log(prob), st)
    fst.SetOutputSymbols(symtab)
    fst.SetInputSymbols(symtab)
    return fst

def build_class_lmfst(lm, probdef, use_phi=False):
    """
    Build an FST from a class-based language model.  By default this
    returns the lazy composition of the class definition transducer
    and the language model.  To obtain the full language model, create
    a VectorFst from it and project it to its input.
    """
    lmfst = build_lmfst(lm, use_phi)
    classfst = build_classfst(probdef, lmfst.InputSymbols())
    openfst.ArcSortInput(lmfst)
    openfst.ArcSortInput(classfst)
    return openfst.StdComposeFst(classfst, lmfst)

def build_dictfst(lmfst):
    """
    Build a character-to-word FST based on the symbol table of lmfst.
    """
    insym = openfst.SymbolTable("letters")
    insym.AddSymbol("&epsilon;")
    outsym = lmfst.InputSymbols()
    fst = openfst.StdVectorFst()
    start = fst.AddState()
    fst.SetStart(start)
    final = fst.AddState()
    fst.SetFinal(final, 0)

    for w, wsym in outsym:
        if wsym == 0: continue
        # Use a single symbol for end-of-sentence
        if w == '</s>':
            w = [w,]
        for c in w:
            csym = insym.AddSymbol(c)

    for w, wsym in outsym:
        if wsym == 0: continue
        wsym = outsym.Find(w)
        # Add an epsilon:word arc to the first state of this word
        prev = fst.AddState()
        fst.AddArc(start, openfst.StdArc(0, wsym, 0, prev))
        # Use a single symbol for end-of-sentence
        if w == '</s>':
            w = [w,]
        for c in w:
            csym = insym.Find(c)
            next = fst.AddState()
            fst.AddArc(prev, openfst.StdArc(csym, 0, 0, next))
            prev = next
        # And an epsilon arc to the final state
        fst.AddArc(prev, openfst.StdArc(0, 0, 0, final))
    fst.SetInputSymbols(insym)
    fst.SetOutputSymbols(outsym)
    return fst

def fst2pdf(fst, outfile, acceptor=False):
    """
    Draw an FST as a PDF using fstdraw and dot.
    """
    tempdir = tempfile.mkdtemp()
    fstfile = os.path.join(tempdir, "output.fst")
    fst.Write(fstfile)
    if acceptor:
        acceptor = "--acceptor"
    else:
        acceptor = ""
    rv = os.system("fstdraw %s '%s' | dot -Tpdf > '%s'"
                   % (acceptor, fstfile, outfile))
    os.unlink(fstfile)
    os.rmdir(tempdir)
    return rv

def sent2fst(txt, fstclass=openfst.StdVectorFst, isyms=None, omitstart=True):
    """
    Convert a list of words, or a string of whitespace-separated
    tokens, to a sentence FST.
    """
    fst = fstclass()
    start = fst.AddState()
    fst.SetStart(start)
    if isyms:
        symtab = isyms
    else:
        symtab = openfst.SymbolTable("words")
        symtab.AddSymbol("&epsilon;")
    prev = start
    if isinstance(txt, str):
        txt = txt.split()
    for c in txt:
        if omitstart and c == '<s>':
            continue
        nxt = fst.AddState()
        if isyms:
            sym = isyms.Find(c)
            if sym == -1:
                #print "Warning, unknown word", c
                continue
        else:
            sym = symtab.AddSymbol(c)
        #print prev, sym, nxt
        fst.AddArc(prev, sym, sym, 0, nxt)
        prev = nxt
    fst.SetFinal(nxt, 0)
    fst.SetInputSymbols(symtab)
    fst.SetOutputSymbols(symtab)
    return fst

def str2fst(txt, fstclass=openfst.StdVectorFst):
    """
    Convert a text string to an FST.
    """
    fst = fstclass()
    start = fst.AddState()
    fst.SetStart(start)
    symtab = openfst.SymbolTable("chars")
    symtab.AddSymbol("&epsilon;")
    prev = start
    for c in txt:
        nxt = fst.AddState()
        sym = symtab.AddSymbol(c)
        fst.AddArc(prev, sym, sym, 0, nxt)
        prev = nxt
    fst.SetFinal(nxt, 0)
    fst.SetInputSymbols(symtab)
    fst.SetOutputSymbols(symtab)
    return fst

def strset2fst(strs, fstclass=openfst.StdVectorFst):
    """
    Build a dictionary lookup FST for a set of strings.
    """
    fst = fstclass()
    isyms = openfst.SymbolTable("chars")
    osyms = openfst.SymbolTable("words")
    isyms.AddSymbol("&epsilon;")
    osyms.AddSymbol("&epsilon;")
    start = fst.AddState()
    fst.SetStart(start)
    for s in strs:
        prev = start
        for c in s:
            nxt = fst.AddState()
            isym = isyms.AddSymbol(c)
            fst.AddArc(prev, isym, 0, 0, nxt)
            prev = nxt
        nxt = fst.AddState()
        osym = osyms.AddSymbol(s)
        fst.AddArc(prev, 0, osym, 0, nxt)
        fst.SetFinal(nxt, 0)
    dfst = fstclass()
    openfst.Determinize(fst, dfst)
    openfst.RmEpsilon(dfst)
    dfst.SetInputSymbols(isyms)
    dfst.SetOutputSymbols(osyms)
    return dfst

import math
def lmfst_eval(lmfst, sent):
    sentfst = sent2fst(sent, openfst.StdVectorFst, lmfst.InputSymbols())
    phi = lmfst.InputSymbols().Find("&phi;")
    if phi != -1:
        opts = openfst.StdPhiComposeOptions()
        opts.matcher1 = openfst.StdPhiMatcher(sentfst, openfst.MATCH_NONE)
        opts.matcher2 = openfst.StdPhiMatcher(lmfst, openfst.MATCH_INPUT, phi)
        c = openfst.StdComposeFst(sentfst, lmfst, opts)
    else:
        c = openfst.StdComposeFst(sentfst, lmfst)
    o = openfst.StdVectorFst()
    openfst.ShortestPath(c, o, 1)
    st = o.Start()
    ll = 0
    while st != -1 and o.NumArcs(st):
        a = o.GetArc(st, 0)
#        print o.InputSymbols().Find(a.ilabel), \
#              o.OutputSymbols().Find(a.olabel), \
#              -a.weight.Value() / math.log(10)
        ll -= a.weight.Value()
        st = a.nextstate
    return ll

def lm_eval(lm, sent):
    sent = [x for x in sent.split() if not x.startswith('++')]
    ll = 0
    for i in xrange(len(sent)):
        if sent[i] == '<s>':
            continue
        prob = lm.prob(sent[i::-1])
        #print sent[i::-1], prob / math.log(10), bo
        ll += prob
    return ll
    
if __name__ == '__main__':
    lmf, fstf = sys.argv[1:]
    lm = sphinxbase.NGramModel(lmf)
    fst = build_lmfst(lm)
    fst.Write(fstf)
