# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Compute Sphinx-III 1s_c_d_dd features.

This module provides a compute() function to compute 39-dimensional
single-stream features consisting of cepstra and their first and
second time derivatives.  This is the default feature type used for
Sphinx-III continuous density acoustic models.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from numpy import zeros, concatenate

def compute(mfcc):
    """Compute 39-dimensional single-stream features consisting of cepstra
     and their first and second time derivatives."""
    r, c = mfcc.shape

    # Pad the features to compute dcep, ddcep
    pad = concatenate((zeros((3,c)), mfcc, zeros((3,c))))

    # Now create the output array
    feat = zeros((r, c*3),'d')

    # cep
    feat[:, 0:13] = mfcc
    
    # dcep = mfc[2] - mfc[-2]
    feat[:, 13:26] = (pad[5:-1] - pad[1:-5])

    # ddcep = (mfc[3] - mfc[-1]) - (mfc[1] - mfc[-3])
    feat[:, 26:] = (pad[6:] - pad[2:-4]) - (pad[4:-2] - pad[0:-6])

    return feat

