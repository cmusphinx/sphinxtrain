#!/usr/bin/env python

import sys
import os
import lattice
from itertools import izip

ctl, ref, latdir = sys.argv[1:]

ctl = open(ctl)
ref = open(ref)
wordcount = 0
errcount = 0
for c,r in izip(ctl, ref):
    c = c.strip()
    r = r.split()
    del r[-1]
    if r[0] != '<s>': r.insert(0, '<s>')
    if r[-1] != '</s>': r.append('</s>')
    l = lattice.Dag()
    l.sphinx2dag(os.path.join(latdir, c + ".lat.gz"))
    err, bt = l.minimum_error(r)
    maxlen = [max([len(y) for y in x]) for x in bt]
    print " ".join(["%*s" % (m, x[0]) for m, x in izip(maxlen, bt)])
    print " ".join(["%*s" % (m, x[1]) for m, x in izip(maxlen, bt)])
    print "Error: %.2f%%" % (float(err) / len(r) * 100)
    print
    wordcount += len(r)
    errcount += err

print "TOTAL Error: %.2f%%" % (float(errcount) / wordcount * 100)
