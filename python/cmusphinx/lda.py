#!/usr/bin/env python

"""
Calculate Fisher's linear discriminant for acoustic models.

This module implements Linear Discriminant Analysis for single
stream Sphinx-III acoustic models.
"""

# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

import sys, os

try:
    import numpy
except ImportError:
    print "FATAL: Failed to import numpy modules. Check that numpy and scipy are installed"
    sys.exit(1)

import s3lda
import s3gaucnt
import itertools

def makelda(gauden_counts):
    """
    Calculate an LDA matrix from a set of mean/full-covariance
    counts as output by the 'bw' program from SphinxTrain.

    @param gauden_counts: Set of full covariance occupation counts.
    @type gauden_counts: cmusphinx.s3gaucnt.S3FullGauCntFile
    """
    if not gauden_counts.pass2var:
        raise Exception, "Please re-run bw with '-2passvar yes'"
    mean = numpy.concatenate([x[0] for x in gauden_counts.mean])
    var = numpy.concatenate([x[0] for x in gauden_counts.var])
    dnom = gauden_counts.dnom.ravel()

    # If CMN was used, this should actually be very close to zero
    globalmean = mean.sum(0) / dnom.sum()
    sw = var.sum(0)
    sb = numpy.zeros(var[0].shape, 'd')
    for d, m in itertools.izip(dnom, mean):
        diff = m / d - globalmean
        sb += d * numpy.outer(diff, diff)

    print "Sw:\n", sw
    print "Sb:\n", sb
    BinvA = numpy.dot(numpy.linalg.inv(sw), sb)
    u, v = numpy.linalg.eig(BinvA)
    
    top = list(u.argsort())
    top.reverse()
    u = u.take(top)
    # Remember, the eigenvalues are in the columns, but Sphinx expects
    # them to be in the rows.
    v = v.T.take(top, 0)

    print "u:\n", u
    print "v:\n", v

    return v

if __name__ == '__main__':
    if len(sys.argv) < 3:
        sys.stderr.write("Usage: %s OUTFILE ACCUMDIRS...\n" % (sys.argv[0]))
        sys.exit(1)

    ldafn = sys.argv[1]
    accumdirs = sys.argv[2:]
    gauden = s3gaucnt.accumdirs_full(accumdirs)
    lda = makelda(gauden)
    s3lda.open(ldafn, 'w').writeall(lda[numpy.newaxis,:])
