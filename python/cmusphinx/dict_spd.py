#!/usr/bin/env python
#
# Copyright (c) 2009 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
Generate a single-pronunciation dictionary from an input dictionary
and the output of force alignment.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision $"

from collections import defaultdict
import s3dict
import sys

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print>>sys.stderr, "Usage: %s INDICT FALIGNOUT [OUTDICT]" % sys.argv[0]
        sys.exit(1)
    indict = s3dict.open(sys.argv[1])
    counts = defaultdict(int)
    falignout = file(sys.argv[2])
    for spam in falignout:
        for word in spam.split()[:-1]:
            if word in indict:
                counts[word] += 1
    words = list(indict.words())
    words.sort()
    if len(sys.argv) > 3:
        outfh = file(sys.argv[3], "w")
    else:
        outfh = sys.stdout
    for w in words:
        alts = sum(1 for x in indict.alts(w))
        if alts == 1:
            print>>outfh, "%s\t\t%s" % (w, " ".join(indict[w]))
        else:
            bestalt = None
            bestcount = 0
            for a in range(1, alts+1):
                if a == 1:
                    wstr = w
                else:
                    wstr = "%s(%d)" % (w, a)
                if counts[wstr] > bestcount:
                    bestcount = counts[wstr]
                    bestalt = wstr
            if bestalt == None:
                print>>outfh, "%s\t\t%s" % (w, " ".join(indict[w]))
            else:
                print>>outfh, "%s\t\t%s" % (w, " ".join(indict[bestalt]))

