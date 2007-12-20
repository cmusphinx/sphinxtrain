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
    """
    A word or sentence HMM, represented as a directed acyclic graph
    of phoneme HMMs.  This object implements the same interface as
    individual HMMs for training, evaluation, and decoding.
    """
    def __init__(self, *hmms):
        self.hmms = []
        self.offsets = [0]
        self.hmmmap = {}
        if hmms:
            self.append(*hmms)

    def append(self, *hmms):
        """
        Append a list of HMMs or tuples of alternative HMMs (more
        interesting graph structures are possible, but not through
        this interface, yet).

        Each argument is either a single sphinx.hmm.HMM object or a
        tuple of multiple HMM objects.  In the latter case, the
        multiple HMMs will be added as alternatives with equal
        transition probabilities into them.
        """
        for h1,h2 in izip(hmms[:-1],hmms[1:]):
            if isinstance(h1, tuple):
                for h in h1:
                    self.hmms.append(h)
                    self.hmmmap[h] = self.offsets[-1]
                    self.offsets.append(self.offsets[-1] + len(h))
                    h.link(h2)
            else:
                self.hmms.append(h1)
                self.hmmmap[h1] = self.offsets[-1]
                self.offsets.append(self.offsets[-1] + len(h1))
                h1.link(h2)
        if isinstance(hmms[-1], tuple):
            for h in hmms[-1]:
                self.hmms.append(h)
                self.hmmmap[h] = self.offsets[-1]
                self.offsets.append(self.offsets[-1] + len(h))
            self.offsets.pop() # Remove the extra offset
        else:
            self.hmms.append(hmms[-1])
            self.hmmmap[hmms[-1]] = self.offsets[-1]

    def get_hmm_idx(self, idx):
        """
        Get HMM and offset from state ID.

        @param idx: State ID in this graph
        @type idx: int
        @return: HMM object and ID of first state in HMM
        @rtype: (sphinx.hmm.HMM, int)
        """
        i = bisect.bisect(self.offsets, idx)
        return self.hmms[i-1], idx - self.offsets[i-1]

    def senid(self, idx):
        """
        Get senone ID from state ID.

        @param idx: State ID in this graph
        @type idx: int
        @return: Senone ID for this state
        @rtype: int
        """
        hmm, offset = self.get_hmm_idx(idx)
        return hmm[offset]

    def tprob(self, i, j):
        """
        Get transition probability from state i to state j.

        @param i: State ID for source state.
        @type i: int
        @param j: State ID for destination state.
        @type j: int
        @return: Transition log-probability (base e) from i to j.
        @rtype: float
        """
        ihmm, ioff = self.get_hmm_idx(i)
        jhmm, joff = self.get_hmm_idx(j)
        if ihmm == jhmm:
            return ihmm[ioff,joff]
        elif ioff == len(ihmm)-1 and joff == 0 and jhmm in ihmm.links:
            return ihmm.links[jhmm]
        else:
            return 0

    def __getitem__(self, key):
        """
        Index this object.

        @param key: Either a single value, in which case the senone ID
                    for the given state ID is returned, or a tuple
                    (i,j), in which case the transition probability
                    from i to j is returned.
        """
        if isinstance(key, tuple):
            return self.tprob(*key)
        else:
            return self.senid(key)

    def iter_senones(self):
        """
        Iterate over senone IDs (in arbitrary order).

        @return: A generator over all senone IDs in this HMMGraph.
        @rtype: generator(int)
        """
        for h in self.hmms:
            for s in h.iter_senones():
                yield s

    def senones(self):
        """
        Return all senone IDs (in arbitrary order).

        @return: All senones in this HMMGraph
        @rtype: (int)
        """
        return tuple(self.iter_senones())

    def __len__(self):
        """Number of states in this HMM graph."""
        return sum([len(h) for h in self.hmms])

    def iter_statepairs(self):
        """
        Iterate over state pairs with non-zero transition
        probabilities.

        @return: A generator over state pairs in this HMMGraph.
                 Transitions out of non-emitting states are returned
                 first, followed by transitions between emitting
                 states, then finally transitions into non-emitting
                 states.
        @rtype: generator((int,int))
        """
        # Transitions out of non-emitting states come first (see
        # below)
        for hmm, offset in izip(self.hmms, self.offsets):
            for ohmm in hmm.links:
                # Transition from final state of this one to first
                # state of successor HMM
                yield offset + len(hmm) - 1, self.hmmmap[ohmm]
        # Transitions into non-emitting states come last (this should
        # happen automatically since they are always last in each
        # state sequence)
        for hmm, offset in izip(self.hmms, self.offsets):
            for i,j in hmm.iter_statepairs():
                yield i + offset, j + offset

class HMM(object):
    """Class representing a single HMM"""
    def __init__(self, sseq, tmat, name=None):
        self.sseq = sseq
        self.tmat = tmat
        self.name = name
        self.links = {}

    def senid(self, idx):
        """
        Get senone ID from state ID.

        @param idx: State ID in this graph
        @type idx: int
        @return: Senone ID for this state
        @rtype: int
        """
        return self.sseq[idx]

    def tprob(self, i, j):
        """
        Get transition probability from state i to state j.

        @param i: State ID for source state.
        @type i: int
        @param j: State ID for destination state.
        @type j: int
        @return: Transition log-probability (base e) from i to j.
        @rtype: float
        """
        return self.tmat[i,j]

    def link(self, others, prob=1.0):
        """
        Add a link to one or more HMMs with total probability prob.
        If others is a tuple, prob will be divided uniformly among
        them (for the time being this is the only way).
        """
        if isinstance(others, tuple):
            for o in others:
                self.link(o, prob/len(others))
        else:
            self.links[others] = prob

    def __getitem__(self, key):
        """
        Index this object.

        @param key: Either a single value, in which case the senone ID
                    for the given state ID is returned, or a tuple
                    (i,j), in which case the transition probability
                    from i to j is returned.
        """
        if isinstance(key, tuple):
            return self.tprob(*key)
        else:
            return self.senid(key)

    def iter_senones(self):
        """
        Iterate over senone IDs (in arbitrary order).

        @return: A generator over all senone IDs in this HMM.
        @rtype: generator(int)
        """
        for s in self.sseq:
            if s != -1:
                yield s

    def senones(self):
        """
        Return all senone IDs (in arbitrary order).

        @return: All senones in this HMM.
        @rtype: (int)
        """
        return tuple(self.iter_senones())

    def __len__(self):
        """Number of states in this HMM."""
        return len(self.sseq)

    def iter_statepairs(self):
        """
        Iterate over state pairs with non-zero transition
        probabilities.

        @return: A generator over state pairs in this HMM.
                 Transitions between emitting states are returned
                 first, followed by transitions into non-emitting
                 states.
        @rtype: generator((int,int))
        """
        return iter(numpy.transpose(self.tmat.nonzero()))

def forward_evaluate(hmm, gmms, alpha=None):
    """
    Calculate the forward variable \\alpha over an HMM or HMMGraph
    for a frame of observations.  The forward variable is defined as::

      \\alpha_t(j) = P(o_1, ..., o_j, q_t = j | \lambda)
      \\alpha_0(0) = 1.0
      \\alpha_t(j) = \sum_i \\alpha_{t-1}(i) a_{ij} b_j(o_t)

    Or, for non-emitting states j_N::

      \\alpha_t(j_N) = \sum_i \\alpha_{t}(i) a_{ij_N}

    Note that non-emitting states transition from the current frame,
    and thus we need to fully calculate \\alpha_{t}(i) for all their
    predecessors before calculating their alpha values.  In other
    words we need to make sure that transitions *into* non-emitting
    states are ordered *after* all others.

    @param hmm: HMM or HMMGraph to evaluate forward variable in
    @param gmms: Collection of GMM scores for current frame, indexed
                 by senone ID.
    @param alpha: List of arrays of previous frames' alpha variables,
                  or None to create a new one.
    @type alpha: [numpy.ndarray]
    @return: Updated list of alpha variables
    @rtype: [numpy.ndarray]
    """
    if alpha == None:
        alpha = numpy.zeros(len(hmm))
        alpha[0] = 1. # Assume unique initial state
    new_alpha = numpy.zeros(len(alpha))
    for i,j in hmm.iter_statepairs():
        if hmm[j] == -1:
            new_alpha[j] += new_alpha[i] * hmm[i,j]
        else:
            new_alpha[j] += alpha[i] * hmm[i,j] * gmms[hmm[j]]
    return new_alpha

def backward_evaluate(hmm, gmms, beta=None):
    """
    Calculate the backward variable \\beta over an HMM or HMMGraph
    for a frame of observations.  The backward variable is defined as::

      \\beta_t(i) = P(o_{t+1}, ..., o_T | q_t = i, \lambda)
      \\beta_T(i) = 1.0 for all final states i
      \\beta_t(i) = \sum_j \\beta_{t+1}(j) a_{ij} b_j(o_t+1)

    Or, for non-emitting states i_N::
    
      \\beta_t(i_N) = \sum_j\\beta_{t}(j) a_{i_Nj} b_j(o_{t})

    Since we only have access to one frame of emissions at a time,
    this means that we must calculate beta_{t+1}(i) for non-emitting
    states in the same pass as beta_{t}(i) for emitting states, but
    before any of them.

    By comparison with forward_evaluate, here we need to make sure
    that transitions *out* of non-emitting states are ordered
    *before* all others.  Luckily that is not in conflict with the
    needs of forward evaluation and a single iter_statepairs() will
    work for both.

    @param hmm: HMM or HMMGraph to evaluate backward variable in
    @param gmms: Collection of GMM scores for current frame, indexed
                 by senone ID.
    @param beta: List of arrays of following frames' beta variables,
                 or None to create a new one.
    @type beta: [numpy.ndarray]
    @return: Updated list of beta variables
    @rtype: [numpy.ndarray]
    """
    new_beta = numpy.zeros(len(hmm))
    # FIXME: For some reason these will break the sum(alpha * beta)
    # invariant if we include them in the beta array.  Also, we don't
    # want to modify the beta argument.  So we store them separately.
    nonemit_beta = numpy.zeros(len(hmm))
    if beta == None:
        beta = numpy.zeros(len(hmm))
        nonemit_beta[-1] = 1. # FIXME: Assumes final state is non-emitting
    for i,j in hmm.iter_statepairs():
        if hmm[i] == -1: # FIXME: Assumes that hmm[j] != -1
            nonemit_beta[i] += beta[j] * hmm[i,j] * gmms[hmm[j]]
        elif hmm[j] == -1:
            new_beta[i] += nonemit_beta[j] * hmm[i,j]
        else:
            new_beta[i] += beta[j] * hmm[i,j] * gmms[hmm[j]]
    return new_beta

class PhoneHMMFactory(object):
    """
    Create single phone (triphone, etc) HMMs.
    """
    def __init__(self, acmod):
        """
        Build a PhoneHMMFactory.

        @param acmod: Acoustic model containing HMM definitions
        @type acmod: sphinx.s3model.S3Model
        """
        self.acmod = acmod

    def create(self, ci, lc='-', rc='-', wpos=None):
        """
        Create an HMM for a triphone (ci, lc, rc, wpos)

        @param ci: Base (context-independent) phone name
        @type ci: string
        @param lc: Left context phone name (or '-' for none)
        @type lc: string
        @param rc: Right context phone name (or '-' for none)
        @type rc: string
        @param wpos: Word position, one of:

          - i: Word-internal phone
          - b: Word-initial phone
          - e: Word-final phone
          - s: Single-phone word (both initial and final)
        
        @type wpos: string
        """
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
        """
        Create a sentence HMM from a list of words.

        FIXME: not implemented yet!

        @param words: sequence of word names
        @type words: (string)
        """
        # Parse words into a phone sequence
        # Build phone HMMs and concatenate them
