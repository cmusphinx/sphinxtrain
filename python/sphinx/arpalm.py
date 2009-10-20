# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read ARPA-format language models.

This module provides a class for reading, writing, and using ARPA
format statistical language model files.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

import numpy
import gzip
import re

LOG10TOLOG = numpy.log(10)
LOGTOLOG10 = 1./LOG10TOLOG

class ArpaLM(object):
    "Class for reading ARPA-format language models"

    def __init__(self, path=None, lw=1.0, wip=1.0):
        """
        Initialize an ArpaLM object.

        @param path: Path to an ARPA format file to (optionally) load
                     language model from.  This file can be
                     gzip-compressed if you like.
        @type path: string
        """
        if path != None:
            self.read(path)
        self.lw = lw
        self.wip = wip
        self.log_wip = numpy.log(wip)

    def read(self, path):
        """
        Load an ARPA format language model from a file in its entirety.

        @param path: Path to an ARPA format file to (optionally) load
                     language model from.  This file can be
                     gzip-compressed if you like.
        @type path: string
        """
        try:
            fh = gzip.open(path)
            fh.readline()
            fh.seek(0,0)
        except:
            fh = file(path)
        # Skip header
        while True:
            spam = fh.readline().rstrip()
            if spam == "\\data\\":
                break

        # Get N-gram counts
        self.ng_counts = {}
        r = re.compile(r"ngram (\d+)=(\d+)")
        while True:
            spam = fh.readline().rstrip()
            if spam == "":
                break
            m = r.match(spam)
            if m != None:
                n, c = map(int, m.groups())
                self.ng_counts[n] = c

        # Word and N-Gram to ID mapping
        self.ngmap = []
        # Create probability/backoff arrays
        self.n = max(self.ng_counts.keys())
        self.ngrams = []
        for n in range(1,self.n+1):
            vals = numpy.zeros((self.ng_counts[n],2),'d')
            self.ngrams.append(vals)
            self.ngmap.append({})

        # Read unigrams and create word id list
        spam = fh.readline().rstrip()
        if spam != "\\1-grams:":
            raise Exception, "1-grams marker not found"
        # ID to word mapping
        self.widmap = []
        wordid = 0
        while True:
            spam = fh.readline().rstrip()
            if spam == "":
                break
            p,w,b = spam.split()
            self.ngmap[0][w] = wordid
            self.widmap.append(w)
            self.ngrams[0][wordid,:] = (float(p) * LOG10TOLOG,
                                        float(b) * LOG10TOLOG)
            wordid = wordid + 1
        
        # Read N-grams
        r = re.compile(r"\\(\d+)-grams:")
        ngramid = 0
        # Successor list map
        self.succmap = {}
        while True:
            spam = fh.readline().rstrip()
            if spam == "":
                continue
            if spam == "\\end\\":
                break
            m = r.match(spam)
            if m != None:
                n = int(m.group(1))
                ngramid = 0
            else:
                spam = spam.split()
                p = float(spam[0]) * LOG10TOLOG
                if n == self.n:
                    ng = tuple(spam[1:])
                    b = 0.0
                else:
                    ng = tuple(spam[1:-1])
                    b = float(spam[-1]) * LOG10TOLOG
                # N-Gram info
                self.ngrams[n-1][ngramid,:] = p, b
                self.ngmap[n-1][ng] = ngramid

                # Successor list for N-1-Gram
                mgram = tuple(ng[:-1])
                if mgram not in self.succmap:
                    self.succmap[mgram] = []
                self.succmap[mgram].append(ng[-1])
                ngramid = ngramid + 1

    def save(self, path):
        """
        Save an ARPA format language model to a file.

        @param path: Path to save the file to.  If this ends in '.gz',
                     the file contents will be gzip-compressed.
        @type path: string
        """
        if path.endswith('.gz'):
            fh = gzip.open(path, 'w')
        else:
            fh = open(path, 'w')
        fh.write("# Written by arpalm.py\n")
        fh.write("\\data\\\n")
        for n in range(1, self.n+1):
            fh.write("ngram %d=%d\n" % (n, self.ng_counts[n]))
        for n in range(1, self.n+1):
            fh.write("\n\\%d-grams:\n" % n)
            ngrams = self.ngmap[n-1].keys()
            ngrams.sort()
            if '<UNK>' in self.ngmap[n-1]:
                ngid = self.ngmap[n-1]['<UNK>']
                score, bowt = self.ngrams[n-1][ngid]
                score *= LOGTOLOG10
                bowt *= LOGTOLOG10
                if n == self.n:
                    fh.write("%.4f <UNK>\n" % (score))
                else:
                    fh.write("%.4f <UNK>\t%.4f\n" % (score,bowt))
            for g in ngrams:
                if g == '<UNK>':
                    continue
                ngid = self.ngmap[n-1][g]
                score, bowt = self.ngrams[n-1][ngid]
                score *= LOGTOLOG10
                bowt *= LOGTOLOG10
                if n > 1:
                    g = " ".join(g) 
                if n == self.n:
                    fh.write("%.4f %s\n" % (score, g))
                else:
                    fh.write("%.4f %s\t%.4f\n" % (score, g, bowt))
        fh.write("\n\\end\\\n")
        fh.close()

    def successors(self, *syms):
        """
        Return all successor words for an M-Gram

        @return: The list of end words for all (M+1)-Grams begining
                 with the words given.
        @rtype: [string]
        """
        try:
            return self.succmap[syms]
        except:
            return []

    def score(self, *syms):
        p = self.prob(*syms)
        return p * self.lw + self.log_wip

    def prob(self, *syms):
        """
        Return the language model log-probability for an N-Gram

        @return: The log probability for the N-Gram consisting of the
                 words given, in base e (natural log).
        @rtype: float
        """
        # It makes the most sense to do this recursively
        n = len(syms)
        if n == 1:
            if syms[0] in self.ngmap[0]:
                # 1-Gram exists, just return its probability
                return self.ngrams[0][self.ngmap[0][syms[0]]][0]
            else:
                # Use <UNK>
                return self.ngrams[0][self.ngmap[0]['<UNK>']][0]
        else:
            if syms in self.ngmap[n-1]:
                # N-Gram exists, just return its probability
                return self.ngrams[n-1][self.ngmap[n-1][syms]][0]
            else:
                # Backoff: alpha(history) * probability (N-1-Gram)
                hist = tuple(syms[0:-1])
                syms = syms[1:]
                # Treat unigram histories a bit specially
                if n == 2:
                    hist = hist[0]
                    # Back off to <UNK> if word doesn't exist
                    if not hist in self.ngmap[0]:
                        hist = '<UNK>'
                if hist in self.ngmap[n-2]:
                    # Try to use the history if it exists
                    bowt = self.ngrams[n-2][self.ngmap[n-2][hist]][1]
                    return bowt + self.score(*syms)
                else:
                    # Otherwise back off some more
                    return self.score(*syms)

    def adapt_rescale(self, unigram, vocab=None):
        """Update unigram probabilities with unigram (assumed to be in
        linear domain), then rescale N-grams ending with the same word
        by the corresponding factors.  If unigram is not the same size
        as the original vocabulary, you must pass vocab, which is a
        list of the words in unigram, in the same order as their
        probabilities are listed in unigram."""
        if vocab:
            # Construct a temporary list mapping for the unigrams
            vmap = map(lambda w: self.ngmap[0][w], vocab)
            # Get the original unigrams
            og = numpy.exp(self.ngrams[0][:,0].take(vmap))
            # Compute the individual scaling factors
            ascale = unigram * og.sum() / og
            # Put back the normalized version of unigram
            self.ngrams[0][:,0].put(numpy.log(unigram * og.sum()), vmap)
            # Now reconstruct vocab as a dictionary mapping words to
            # scaling factors
            vv = {}
            for i, w in enumerate(vocab):
                vv[w] = i
            vocab = vv
        else:
            ascale = unigram / numpy.exp(self.ngrams[0][:,0])
            self.ngrams[0][:,0] = numpy.log(unigram)

        for n in range(1, self.n):
            # Total discounted probabilities for each history
            tprob = numpy.zeros(self.ngrams[n-1].shape[0], 'd')
            # Rescaled total probabilities
            newtprob = numpy.zeros(self.ngrams[n-1].shape[0], 'd')
            # For each N-gram, accumulate and rescale
            for ng,idx in self.ngmap[n].iteritems():
                h = ng[0:-1]
                if n == 1: # Quirk of unigrams
                    h = h[0]
                w = ng[-1]
                prob = numpy.exp(self.ngrams[n][idx,0])
                tprob[self.ngmap[n-1][h]] += prob
                if vocab == None or w in vocab:
                    prob = prob * ascale[vocab[w]]
                newtprob[self.ngmap[n-1][h]] += prob
                self.ngrams[n][idx,0] = numpy.log(prob)
            # Now renormalize everything
            norm = tprob / newtprob
            for ng,idx in self.ngmap[n].iteritems():
                h = ng[0:-1]
                if n == 1: # Quirk of unigrams
                    h = h[0]
                w = ng[-1]
                prob = numpy.exp(self.ngrams[n][idx,0])
                self.ngrams[n][idx,0] = numpy.log(prob * norm[self.ngmap[n-1][h]])
