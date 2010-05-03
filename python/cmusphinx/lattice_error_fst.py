#!/usr/bin/env python

# Copyright (c) 2010  Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
Find lattice word error rate using OpenFST.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"


import sys
import os
import re
import openfst
import fstutils
import lattice
import lat2fsg
from itertools import izip

def is_filler(sym):
    if sym == '<s>' or sym == '</s>': return False
    return ((sym[0] == '<' and sym[-1] == '>') or
            (sym[0] == '+' and sym[-1] == '+'))

linere = re.compile(r"^\s*(?:<s>)?\s*([^(]+)(?:</s>)?\s*(?:\(([^)]+)\))\s*?")
def get_utt(line):
    m = linere.match(line)
    if m:
        return m.groups()
    else:
        return (None, None)

class LevenshteinModel(openfst.StdVectorFst):
    def __init__(self, symtab, scost=1, icost=1, dcost=1):
        openfst.StdVectorFst.__init__(self)
        st = self.AddState()
        self.SetStart(st)
        self.SetFinal(st, 0)
        sigma = symtab.Find("&sigma;")
        for c, val in symtab:
            if val in (openfst.epsilon, sigma):
                continue
            # Translation arc
            self.AddArc(st, val, val, 0, st)
            # Insertion/Deletion arcs
            self.AddArc(st, 0, val, icost, st)
            self.AddArc(st, val, 0, dcost, st)
            # Substitution arcs
            for cc, vv in symtab:
                if vv in (openfst.epsilon, sigma, val):
                    continue
                self.AddArc(st, val, vv, scost, st)
        self.SetInputSymbols(symtab)
        self.SetOutputSymbols(symtab)

class CompoundWordModel(openfst.StdVectorFst):
    def __init__(self, isyms, osyms):
        openfst.StdVectorFst.__init__(self)
        st = self.AddState()
        self.SetStart(st)
        self.SetFinal(st, 0)
        sigma = osyms.Find("&sigma;")
        for c, val in osyms:
            if val in (openfst.epsilon, sigma):
                continue
            # Translation arc
            self.AddArc(st, val, val, 0, st)
            # Compound word separator
            if '_' not in c:
                continue
            parts = c.split('_')
            prev = st
            nx = self.AddState()
            # Transduce word sequence from input
            isym = isyms.AddSymbol(parts[0])
            self.AddArc(prev, isym, openfst.epsilon, 0, nx)
            prev = nx
            for p in parts[1:-1]:
                nx = self.AddState()
                isym = isyms.AddSymbol(p)
                self.AddArc(prev, isym, openfst.epsilon, 0, nx)
                prev = nx
            isym = isyms.AddSymbol(parts[-1])
            # Finally insert compound into output
            self.AddArc(prev, isym, val, 0, st)
        self.SetInputSymbols(isyms)
        self.SetOutputSymbols(osyms)

if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser(usage="%prog CTL REF LATDIR")
    parser.add_option("--prune", type="float")
    opts, args = parser.parse_args(sys.argv[1:])
    
    ctl, ref, latdir = args
    ctl = open(ctl)
    ref = open(ref)
    wordcount = 0
    errcount = 0
    for c,r in izip(ctl, ref):
        # Normalize reference, etc.
        ref, refid = get_utt(r)
        c = c.strip()
        r = ref.split()
        if len(r) == 0 or r[0] != '<s>': r.insert(0, '<s>')
        if r[-1] != '</s>': r.append('</s>')
        r = filter(lambda x: not is_filler(x), r)
        # Turn it into an FSM
        rfst = fstutils.sent2fst(r)
        # Get the hypothesis lattice
        try:
            l = lattice.Dag(os.path.join(latdir, c + ".lat"))
        except IOError:
            try:
                l = lattice.Dag(os.path.join(latdir, c + ".lat.gz"))
            except IOError:
                l = lattice.Dag(htk_file=os.path.join(latdir, c + ".slf"))
        if opts.prune != None:
            l.posterior_prune(-opts.prune)
        # Convert it to an FSM
        lfst = lat2fsg.build_lattice_fsg(l, rfst.OutputSymbols(),
                                         addsyms=True, determinize=False,
                                         baseword=lattice.baseword_noclass)
        openfst.ArcSortInput(lfst)
        # Apply Levenshtein model to the input
        errfst = LevenshteinModel(rfst.OutputSymbols())
        openfst.ArcSortInput(errfst)
        # Apply compound word model based on the lattice
        compfst = CompoundWordModel(errfst.OutputSymbols(),
                                    lfst.InputSymbols())
        # Precompose and project it to the lattice so compound words
        # are split in the alignment
        xlat = openfst.StdVectorFst()
        openfst.Compose(compfst, lfst, xlat)
        openfst.ProjectInput(xlat)
        openfst.ArcSortInput(xlat)
        # Compose everything together
        cfst = openfst.StdComposeFst(rfst, errfst)
        cfst = openfst.StdComposeFst(cfst, xlat)
        # Do bestpath search
        ofst = openfst.StdVectorFst()
        openfst.ShortestPath(cfst, ofst, 1)
        st = ofst.Start()
        err = 0
        bt = []
        while st != -1 and ofst.NumArcs(st):
            a = ofst.GetArc(st, 0)
            isym = ofst.InputSymbols().Find(a.ilabel)
            osym = ofst.OutputSymbols().Find(a.olabel)
            if isym == '</s>':
                break
            if a.ilabel == openfst.epsilon:
                isym = '*INS*'
            if a.olabel == openfst.epsilon:
                osym = '*DEL*'
            bt.append((isym, osym))
            err += a.weight.Value()
            st = a.nextstate
        maxlen = [max([len(y) for y in x]) for x in bt]
        nwords = len(r) - 2
        refid = '(%s)' % refid
        c = '(%s)' % c
        print " ".join(["%*s" % (m, x[0]) for m, x in izip(maxlen, bt)]), refid
        print " ".join(["%*s" % (m, x[1]) for m, x in izip(maxlen, bt)]), c
        if nwords:
            print "Error: %.2f%%" % (float(err) / nwords * 100)
        else:
            print "Error: %.2f%%" % (float(err) * 100)
        print
        wordcount += nwords
        errcount += err

    print "TOTAL Error: %.2f%%" % (float(errcount) / wordcount * 100)
