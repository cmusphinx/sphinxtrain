#!/usr/bin/env python

# Copyright (c) 2010  Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III
"""
Rescore a lattice using a language model directly
"""

__author__ = "David Huggins-Daines <dhdaines@gmail.com>"
__version__ = "$Revision $"

import sphinxbase
from cmusphinx import lattice
import sys
import os


def lat_rescore(latfile, lmfst):
    """
    Rescore a lattice using a language model.
    """
    dag = lattice.Dag(latfile)
    end = dag.bestpath(lm)
    return [lattice.baseword(x.sym) for x in dag.backtrace(end)], end.score


if __name__ == '__main__':
    ctlfile, latdir, lmfile = sys.argv[1:]
    lm = sphinxbase.NGramModel(lmfile, wip=1.0, lw=9.5)
    for spam in open(ctlfile):
        latfile = os.path.join(latdir, spam.strip() + ".lat.gz")
        words, score = lat_rescore(latfile, lm)
        print(" ".join(words), "(%s %f)" % (spam.strip(), score))
