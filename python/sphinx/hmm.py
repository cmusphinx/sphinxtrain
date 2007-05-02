# Copyright (c) 2007 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Hidden Markov Model objects for training/decoding.

This module provides a basic HMM object and factory classes for
building HMMs from triphones and from sentences.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

import numpy
import bisect
from itertools import izip

class HMMGraph(object):
    def __init__(self, *hmms):
        self.hmms = []
        self.offsets = [0]
        if hmms:
            self.append(*hmms)

    def append(self, *hmms):
        """Append a list of HMMs or tuples of alternative HMMs (more
        interesting graph structures are possible, but not through
        this interface, yet)."""
        for h1,h2 in izip(hmms[:-1],hmms[1:]):
            if isinstance(h1, tuple):
                for h in h1:
                    self.hmms.append(h)
                    self.offsets.append(self.offsets[-1] + len(h))
                    h.link(h2)
            else:
                self.hmms.append(h1)
                self.offsets.append(self.offsets[-1] + len(h1))
                h1.link(h2)
        if isinstance(hmms[-1], tuple):
            for h in hmms[-1]:
                self.hmms.append(h)
        else:
            self.hmms.append(hmms[-1])

    def get_hmm_idx(self, idx):
        """Get HMM and offset for state index idx."""
        i = bisect.bisect(self.offsets, idx)
        return self.hmms[i-1], idx - self.offsets[i-1]

    def senid(self, idx):
        """Get senone ID from state ID."""
        hmm, offset = self.get_hmm_idx(idx)
        return hmm[offset]

    def tprob(self, i, j):
        """Get transition probability from state i to state j"""
        ihmm, ioff = self.get_hmm_idx(i)
        jhmm, joff = self.get_hmm_idx(j)
        if ihmm == jhmm:
            return ihmm[ioff,joff]
        elif ioff == len(ihmm)-1 and joff == 0 and jhmm in ihmm.links:
            return ihmm.links[jhmm]
        else:
            return 0

    def iter_senones(self):
        """Iterate over senone IDs (in arbitrary order)."""
        for h in self.hmms:
            for s in h.iter_senones():
                yield s

    def senones(self):
        """Return all senone IDs (in arbitrary order)."""
        return tuple(self.iter_senones())

    def __len__(self):
        """Number of states in this HMM graph."""
        return sum([len(h) for h in self.hmms])

    def iter_statepairs(self):
        """Iterate over state pairs with non-zero transition
        probabilities."""
        

class HMM(object):
    """Class representing a single HMM"""
    def __init__(self, sseq, tmat, name=None):
        self.sseq = sseq
        self.tmat = tmat
        self.name = name
        self.links = {}

    def senid(self, idx):
        """Get senone ID from state ID."""
        return self.sseq[idx]

    def tprob(self, i, j):
        """Get transition probability from state i to state j"""
        return self.tmat[i,j]

    def link(self, others, prob=1.0):
        """Add a link to one or more HMMs with total probability prob.
        If others is a tuple, prob will be divided uniformly among
        them (for the time being this is the only way)."""
        if isinstance(others, tuple):
            for o in others:
                self.link(o, prob/len(others))
        else:
            self.links[others] = prob

    def __getitem__(self, key):
        if isinstance(key, tuple):
            return self.tprob(*key)
        else:
            return self.senid(key)

    def iter_senones(self):
        """Iterate over senone IDs."""
        for s in self.sseq:
            if s != -1:
                yield s

    def senones(self):
        """Return all senone IDs."""
        return tuple(self.iter_senones())

    def __len__(self):
        return len(self.sseq)

    def iter_statepairs(self):
        """Iterate over state ID pairs with nonzero transition probabilities."""
        return iter(numpy.transpose(self.tmat.nonzero()))

def forward_evaluate(hmm, gmms, alpha=None):
    if alpha == None:
        alpha = numpy.zeros(len(hmm))
        alpha[0] = 1. # Assume unique initial state
    new_alpha = numpy.zeros(len(alpha))
    for i,j in hmm.iter_statepairs():
        senid = hmm.senid(j)
        if senid == -1:
            new_alpha[j] += alpha[i] * hmm.tprob(i,j)
        else:
            new_alpha[j] += alpha[i] * hmm.tprob(i,j) * gmms[senid]
    return new_alpha

def backward_evaluate(hmm, gmms, beta=None):
    if beta == None:
        beta = numpy.zeros(len(hmm))
        beta[-1] = 1. # And unique final state
    new_beta = numpy.zeros(len(beta))
    for i,j in hmm.iter_statepairs():
        senid = hmm.senid(j)
        if senid == -1:
            new_beta[i] += beta[j] * hmm.tprob(i,j)
        else:
            new_beta[i] += beta[j] * hmm.tprob(i,j) * gmms[senid]
    return new_beta

class PhoneHMMFactory(object):
    """Create single phone (triphone, etc) HMMs"""
    def __init__(self, acmod):
        self.acmod = acmod

    def create(self, ci, lc='-', rc='-', wpos=None):
        """Create an HMM for a triphone (ci, lc, rc, wpos)"""
        pid = self.acmod.mdef.phone_id(ci, lc, rc, wpos)
        ssid = self.acmod.mdef.pid2ssid(pid)
        return HMM(self.acmod.mdef.sseq[ssid],
                   self.acmod.tmat[self.acmod.mdef.pid2tmat(pid)],
                   (ci, lc, rc, wpos))

class SentenceHMMFactory(object):
    """Create sentence HMMs"""
    def __init__(self, acmod, dictionary):
        self.acmod = acmod
        self.dictionary = dictionary
        self.phone_factory = PhoneHMMFactory(acmod)

    def create(self, words):
        """Create a sentence HMM from a list of words"""
        # Parse words into a phone sequence
        # Build phone HMMs and concatenate them
