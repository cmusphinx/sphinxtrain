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

class HMMGraph(object):
    def __init__(self, *hmms):
        pass

    def copy(self):
        pass

    def senid(self, idx):
        pass

    def tprob(self, i, j):
        pass

    def iter_senones(self):
        pass

    def senones(self):
        pass

    def __len__(self):
        pass

    def iter_statepairs(self):
        pass

class HMM(object):
    """Class representing a single HMM"""
    def __init__(self, sseq, tmat):
        self.sseq = sseq
        self.tmat = tmat
        self.links = []

    def copy(self):
        return self.__class__(self.sseq.copy(), self.tmat.copy())

    def senid(self, idx):
        """Get senone ID from state ID."""
        return self.sseq[idx]

    def tprob(self, i, j):
        """Get transition probability from state i to state j"""
        return self.tmat[i,j]

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

    def create(self, pid, ssid=None):
        """Create an HMM for base phone PID and senone sequence SSID"""
        if ssid == None:
            # Use the CI phone model
            ssid = self.acmod.mdef.pid2ssid(pid)
        return HMM(self.acmod.mdef.sseq[ssid],
                   self.acmod.tmat[self.acmod.mdef.pid2tmat(pid)])

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
