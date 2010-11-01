#!/usr/bin/env python

import sys
import os
import lattice
from itertools import izip

ctl, ref, latdir = sys.argv[1:4]
prune = 0
if len(sys.argv) > 4:
    prune = float(sys.argv[4])

ctl = open(ctl)
ref = open(ref)
wordcount = 0
errcount = 0
for c,r in izip(ctl, ref):
    c = c.strip()
    r = r.split()
    del r[-1]
    if len(r) == 0 or r[0] != '<s>': r.insert(0, '<s>')
    if r[-1] != '</s>': r.append('</s>')
    nw = len(r) - 2
    r = filter(lambda x: not lattice.is_filler(x), r)
    l = lattice.Dag()
    try:
        l.sphinx2dag(os.path.join(latdir, c + ".lat.gz"))
    except IOError:
        try:
            l.sphinx2dag(os.path.join(latdir, c + ".lat"))
        except IOError:
            l.htk2dag(os.path.join(latdir, c + ".slf"))
    if prune:
        l.posterior_prune(-prune)
    err, bt = l.minimum_error(r)
    maxlen = [max([len(y) for y in x]) for x in bt]
    print " ".join(["%*s" % (m, x[0]) for m, x in izip(maxlen, bt)])
    print " ".join(["%*s" % (m, x[1]) for m, x in izip(maxlen, bt)])
    if nw:
        print "Error: %.2f%%" % (float(err) / nw * 100)
    else:
        print "Error: %.2f%%" % 0
    print
    wordcount += nw
    errcount += err

print "TOTAL Error: %.2f%%" % (float(errcount) / wordcount * 100)
