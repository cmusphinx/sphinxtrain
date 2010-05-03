#!/usr/bin/env python

# Copyright (c) 2010  Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
Convert Sphinx lattices to finite state automata and grammars.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

import openfst
import lattice
import math
import sys
import os

def baseword(sym):
    """
    Returns base word (no pronunciation variant) for sym.
    """
    paren = sym.rfind('(')
    if paren != -1:
        return sym[0:paren]
    else:
        return sym

def realword(sym):
    """
    Test if a word is a real word (not silence or filler)
    """
    if sym.lower() in ('<s>','<sil>','</s>'):
        return False
    if sym.startswith("++"):
        return False
    return True

def build_lattice_fsg(dag, syms=None, ascale=0, pscale=0,
                      addsyms=False, determinize=True,
                      baseword=baseword):
    """
    Build an FSM from a Sphinx word lattice.
    """
    fst = openfst.StdVectorFst()
    if syms == None:
        fsgsyms = openfst.SymbolTable("words")
        fsgsyms.AddSymbol("&epsilon;")
        fsgsyms.AddSymbol("&sigma;")
        fsgsyms.AddSymbol("&rho;")
        fsgsyms.AddSymbol("&phi;")
        addsyms = True
    else:
        fsgsyms = syms
    statemap = {}
    j = 0
    for n in dag.nodes:
        # Skip fillers as they have been "bypassed" by PocketSphinx
        if n.sym.startswith("++") or n.sym == "<sil>":
            continue
        # These should not exist, but they do (!!)
        if n.sym == "<s>" and n.entry != 0:
            continue
        if n not in statemap:
            statemap[n] = fst.AddState()
        if addsyms:
            fsgsyms.AddSymbol(baseword(n.sym))
    statemap[dag.start] = fst.AddState()
    fst.SetStart(statemap[dag.start])
    for n in dag.nodes:
        if n not in statemap:
            continue
        sym = fsgsyms.Find(baseword(n.sym))
        for x in n.exits:
            if x.dest not in statemap:
                continue
            weight = 0
            # Turn OOVs and non-events into epsilons
            if sym == -1 or n.sym == "<s>":
                sym = 0
            if ascale:
                weight = -x.ascr * ascale
            elif pscale:
                weight = -x.post * pscale
            fst.AddArc(statemap[x.src], sym, sym, weight, statemap[x.dest])
    # Add a </s> transition if none exists
    if '</s>' not in [x.src.sym for x in dag.end.entries]:
        end = fst.AddState()
        sym = fsgsyms.AddSymbol("</s>")
        fst.AddArc(statemap[dag.end], sym, sym, 0, end)
        fst.SetFinal(end, 0)
    else:
        fst.SetFinal(statemap[dag.end], 0)
    # Epsilon-remove it (like bypassing fillers...) (FIXME: umm...)
    openfst.RmEpsilon(fst)
    # Don't determinize if it's weighted
    if ascale or pscale:
        determinize = False
    if determinize:
        outfst = openfst.StdVectorFst()
        openfst.Determinize(fst, outfst)
        fst = outfst
    fst.SetInputSymbols(fsgsyms)
    fst.SetOutputSymbols(fsgsyms)
    return fst

def build_fsg_fst(fst, outfile, name="fromfst"):
    """
    Build a Sphinx FSG from an FST.
    """
    if not isinstance(outfile, file):
        outfile = file(outfile, "w")
    outfile.write("FSG_BEGIN %s\n" % name)
    outfile.write("NUM_STATES %d\n" % fst.NumStates())
    outfile.write("START_STATE %d\n" % fst.Start())
    # Assume there's only one final state
    end = None
    for state in fst:
        if fst.IsFinal(state):
            end = state
            break
    if end == None:
        raise RuntimeError, "No ending state in lattice, writing %s failed" % outfile
    outfile.write("FINAL_STATE %d\n" % end)
    syms = fst.OutputSymbols()
    for state in fst:
        for arc in fst.iterarcs(state):
            if arc.ilabel == 0 or syms.Find(arc.ilabel) in ('<s>','</s>'):
                outfile.write("TRANSITION %d %d %g\n"
                              % (state, arc.nextstate,
                                 math.exp(-arc.weight.Value())))
            else:
                outfile.write("TRANSITION %d %d %g %s\n"
                              % (state, arc.nextstate,
                                 math.exp(-arc.weight.Value()),
                                 syms.Find(arc.ilabel)))
    outfile.write("FSG_END\n")
    outfile.flush()

def lat2fsg(lat, fsgfile, lmfst, prune=15):
    if isinstance(lat, str):
        if lat.endswith(".slf"):
            dag = lattice.Dag(htkfile=lat)
        else:
            dag = lattice.Dag(lat)
    else:
        dag = lat
    fst = build_lattice_fsg(dag, lmfst.InputSymbols())
    # Compose it (intersect, really) with the language model to get
    # correct N-gram scores (otherwise it is just a unigram LM).  This
    # is the same thing as "lattice expansion".
    phi = lmfst.InputSymbols().Find("&phi;")
    if phi != -1:
        opts = openfst.StdPhiComposeOptions()
        opts.matcher1 = openfst.StdPhiMatcher(fst, openfst.MATCH_NONE)
        opts.matcher2 = openfst.StdPhiMatcher(lmfst, openfst.MATCH_INPUT, phi)
        cfst = openfst.StdComposeFst(fst, lmfst, opts)
    else:
        cfst = openfst.StdComposeFst(fst, lmfst)
    outfst = openfst.StdVectorFst(cfst)
    openfst.Prune(outfst, prune)
    # Write it back out as an FSG for PocketSphinx.
    build_fsg_fst(outfst, fsgfile)
    return outfst

def apply_errfst(fst, errfst):
    sigma = errfst.InputSymbols().Find("&sigma;")
    opts = openfst.StdSigmaComposeOptions()
    opts.matcher1 = openfst.StdSigmaMatcher(fst, openfst.MATCH_NONE)
    opts.matcher2 = openfst.StdSigmaMatcher(errfst, openfst.MATCH_INPUT, sigma, True)
    cfst = openfst.StdComposeFst(fst, errfst, opts)
    cfst = openfst.StdVectorFst(cfst)
    openfst.ProjectOutput(cfst)
    return cfst

def lat2fsg_posterior(lat, fsgfile, prune=5, errfst=None):
    if isinstance(lat, str):
        if lat.endswith(".slf"):
            dag = lattice.Dag(htk_file=lat)
        else:
            dag = lattice.Dag(lat)
    else:
        dag = lat
    dag.posterior_prune(-prune)
    fst = build_lattice_fsg(dag, pscale=1)
    if errfst:
        fst = build_lattice_fsg(dag, errfst.InputSymbols(), pscale=1, addsyms=True)
        errfst.SetOutputSymbols(errfst.InputSymbols())
        fst = apply_errfst(fst, errfst)
    else:
        fst = build_lattice_fsg(dag, pscale=1)
    build_fsg_fst(fst, fsgfile)
    return fst

def lat2flat(latfile, fsgfile, lmfst):
    """
    Subset a language model using the vocabulary of a lattice.
    """
    dag = lattice.Dag(latfile)
    fst = openfst.StdVectorFst()
    fst.SetStart(fst.AddState())
    fst.SetFinal(0, 0)
    syms = lmfst.InputSymbols()
    seen = set()
    for n in dag.nodes:
        # Skip fillers as they have been "bypassed" by PocketSphinx
        if n.sym.startswith("++") or n.sym == "<sil>":
            continue
        if n.sym in seen:
            continue
        seen.add(n.sym)
        sym = syms.Find(baseword(n.sym))
        if sym == -1:
            continue
        fst.AddArc(0, sym, sym, 0, 0)
    fst.SetOutputSymbols(lmfst.InputSymbols())
    phi = lmfst.InputSymbols().Find("&phi;")
    if phi != -1:
        opts = openfst.StdPhiComposeOptions()
        opts.matcher1 = openfst.StdPhiMatcher(fst, openfst.MATCH_NONE)
        opts.matcher2 = openfst.StdPhiMatcher(lmfst, openfst.MATCH_INPUT, phi)
        cfst = openfst.StdComposeFst(fst, lmfst, opts)
    else:
        cfst = openfst.StdComposeFst(fst, lmfst)
    outfst = openfst.StdVectorFst()
    openfst.Determinize(cfst, outfst)
    # Write it back out as an FSG for PocketSphinx.
    build_fsg_fst(outfst, fsgfile)
    return outfst

if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser(usage="%prog CTL LATDIR")
    parser.add_option("--prune", type="int", default=5)
    parser.add_option("--errprune", type="float", default=1e-5)
    parser.add_option("--errpen", type="float", default=1.0)
    parser.add_option("--outext")
    parser.add_option("--errfst")
    opts, args = parser.parse_args(sys.argv[1:])
    ctlfile, latdir = args
    errfst = None
    if opts.errfst:
        errfst = openfst.StdVectorFst.Read(opts.errfst)
        openfst.Prune(errfst, opts.errprune)
        sigma = errfst.InputSymbols().Find("&sigma;")
        for node in errfst:
            for ai in errfst.mutable_iterarcs(node):
                arc = ai.Value()
                if arc.ilabel == sigma:
                    continue
                ai.SetValue(openfst.StdArc(arc.ilabel, arc.olabel, 
                                           arc.weight.Value() + opts.errpen,
                                           arc.nextstate))
        openfst.ArcSortInput(errfst)
    if opts.outext == None:
        opts.outext = ".fsg%d" % opts.prune
    for spam in file(ctlfile):
        latfile = os.path.join(latdir, spam.strip() + ".slf")
        fsgfile = os.path.join(latdir, spam.strip() + opts.outext)
        print spam,
        ofst = lat2fsg_posterior(latfile, fsgfile, opts.prune, errfst)
