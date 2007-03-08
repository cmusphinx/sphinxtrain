# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read ARPA-format language models.

This module provides a class for reading (currently not writing) ARPA
format statistical language model files.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from numpy import zeros
from math import log, exp
import re

class model(object):
    "Class for reading ARPA-format language models"
    def __init__(self, path=None):
        if path != None:
            self.read(path)

    def read(self, path):
        "Load an ARPA format language model from a file in its entirety"
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
            vals = zeros((self.ng_counts[n],2),'d')
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
            self.ngrams[0][wordid,:] = float(p), float(b)
            wordid = wordid + 1
        
        # Read N-grams
        r = re.compile(r"\\(\d+)-grams:")
        ngramid = 0
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
                p = float(spam[0])
                if n == self.n:
                    ng = tuple(spam[1:])
                    b = 0.0
                else:
                    ng = tuple(spam[1:-1])
                    b = float(spam[-1])
                # N-Gram info
                self.ngrams[n-1][ngramid,:] = p, b
                self.ngmap[n-1][ng] = ngramid
                ngramid = ngramid + 1

    def save(self, path):
        "Save an ARPA format language model to a file"
        fh = file(path, 'w')
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
                if n == self.n:
                    fh.write("%.4f <UNK>\n" % (score))
                else:
                    fh.write("%.4f <UNK>\t%.4f\n" % (score,bowt))
            for g in ngrams:
                if g == '<UNK>':
                    continue
                ngid = self.ngmap[n-1][g]
                score, bowt = self.ngrams[n-1][ngid]
                if n > 1:
                    g = " ".join(g) 
                if n == self.n:
                    fh.write("%.4f %s\n" % (score, g))
                else:
                    fh.write("%.4f %s\t%.4f\n" % (score, g, bowt))
        fh.write("\n\\end\\\n")
        fh.close()

    def score(self, *syms):
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
