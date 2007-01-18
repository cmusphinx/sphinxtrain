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
from math import log
import re

LOG10 = log(10)

class ArpaLM(object):
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
                p = float(spam[0])
                if n == self.n:
                    ng = spam[1:]
                    b = 0.0
                else:
                    ng = spam[1:-1]
                    b = float(spam[-1])
                # N-Gram info
                self.ngrams[n-1][ngramid,:] = p, b
                self.ngmap[n-1][" ".join(ng)] = ngramid

                # Successor list for N-1-Gram
                mgram = " ".join(ng[:-1])
                if mgram not in self.succmap:
                    self.succmap[mgram] = []
                self.succmap[mgram].append(ng[-1])
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
            for g in ngrams:
                ngid = self.ngmap[n-1][g]
                score, bowt = self.ngrams[n-1][ngid]
                if n == self.n:
                    fh.write("%.4f %s\n" % (score, g))
                else:
                    fh.write("%.4f %s\t%.4f\n" % (score, g, bowt))
        fh.write("\n\\end\\\n")
        fh.close()

    def ngstr(self, *w):
        "Create the N-gram string for a sequence of IDs"
        return " ".join(widmap[x] for x in w )
    
    def successors(self, *syms):
        "Find all successors for an N-1-gram"
        try:
            return self.succmap[" ".join(syms)]
        except:
            return []

    def score(self, *syms):
        "Return the log probability score for an N-gram"
        n = len(syms)
        while n > 0:
            ng = " ".join(syms[-n:])
            try:
                ngid = self.ngmap[n-1][ng]
                score, bowt = self.ngrams[n-1][ngid]
                # Convert to natural log
                return (bowt + score) * LOG10
            except:
                n = n - 1
        return -99 * LOG10 # very improbable
