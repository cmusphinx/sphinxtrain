#!/usr/bin/env python

# Copyright (c) 2010  Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
Rescore a lattice using a language model FST (or a set of them).
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision $"


import openfst
import lattice
import lat2fsg
import math
import sys
import os

def lat_rescore(dag, lmfst, lw=9.5):
    """
    Rescore a lattice using a language model FST.
    """
    fst = lat2fsg.build_lattice_fsg(dag, lmfst.InputSymbols(), 1./lw)
    phi = lmfst.InputSymbols().Find("&phi;")
    if phi != -1:
        opts = openfst.StdPhiComposeOptions()
        opts.matcher1 = openfst.StdPhiMatcher(fst, openfst.MATCH_NONE)
        opts.matcher2 = openfst.StdPhiMatcher(lmfst, openfst.MATCH_INPUT, phi)
        c = openfst.StdComposeFst(fst, lmfst, opts)
    else:
        c = openfst.StdComposeFst(fst, lmfst)
    o = openfst.StdVectorFst()
    openfst.ShortestPath(c, o, 1)
    words = ['<s>']
    st = o.Start()
    score = 0
    while st != -1 and o.NumArcs(st):
        a = o.GetArc(st, 0)
        if a.olabel != 0:
            words.append(lmfst.InputSymbols().Find(a.ilabel))
        score -= a.weight.Value()
        st = a.nextstate
    return words, score

if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser(usage="%prog CTL LATDIR [LMFST]")
    parser.add_option("--lmnamectl")
    parser.add_option("--lmdir", default=".")
    parser.add_option("--lw", type="float", default=7)
    opts, args = parser.parse_args(sys.argv[1:])
    ctlfile, latdir = args[0:2]
    if len(args) > 2:
        lmfst = openfst.StdVectorFst.Read(args[2])
        lmnamectl = None
    elif opts.lmnamectl:
        lmnamectl = file(opts.lmnamectl)
        lmfsts = {}
    else:
        parser.error("either --lmnamectl or LMFST must be given")
    for spam in file(ctlfile):
        if lmnamectl:
            lmname = lmnamectl.readline().strip()
            if lmname not in lmfsts:
                lmfsts[lmname] = openfst.StdVectorFst.Read(os.path.join(opts.lmdir,
                                                                        lmname + ".arpa.fst"))
            lmfst = lmfsts[lmname]
        try:
            dag = lattice.Dag(os.path.join(latdir, spam.strip() + ".lat.gz"))
        except IOError:
            try:
                dag = lattice.Dag(os.path.join(latdir, spam.strip() + ".lat"))
            except IOError:
                dag = lattice.Dag(htk_file=os.path.join(latdir, spam.strip() + ".slf"))
        words, score = lat_rescore(dag, lmfst, opts.lw)
        print " ".join(words), "(%s %f)" % (spam.strip(), score)
