# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Compute Sphinx-II 4-stream features.

This module provides a compute() function to compute 52-dimensional
4-stream features consisting of cepstra and their first and second
time derivatives, and the C0 values. This is the default feature type
used for PocketSphinx and Sphinx-II semi-continuous acoustic models.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from numpy import zeros, concatenate

def compute(mfcc):
    """Compute 52-dimensional 4-stream features consisting of cepstra
     and their first and second time derivatives, and the C0 values."""
    r, c = mfcc.shape

    # Pad the features to compute dcep, ddcep
    pad = concatenate((zeros((5,c)), mfcc, zeros((5,c))))

    # Now create the output array
    feat = (zeros((r, c-1),'d'),
            zeros((r, (c-1)*2),'d'),
            zeros((r, 3),'d'),
            zeros((r, c-1),'d'))

    # cep (without C0)
    feat[0][:] = mfcc[:,1:]
    
    # dcep = mfc[2] - mfc[-2] : mfc[4] - mfc[-4]
    feat[1][:, 0:12] = (pad[7:-3,1:] - pad[3:-7,1:])
    feat[1][:, 12:]  = (pad[9:-1,1:] - pad[1:-9,1:])

    # ddcep = (mfc[3] - mfc[-1]) - (mfc[1] - mfc[-3])
    feat[3][:] = (pad[8:-2,1:] - pad[4:-6,1:]) - (pad[6:-4,1:] - pad[2:-8,1:])

    # pow = C0,D0,DD0
    feat[2][:,0] = mfcc[:,0]
    feat[2][:,1] = (pad[7:-3,0] - pad[3:-7,0])
    feat[2][:,2] = (pad[8:-2,0] - pad[4:-6,0]) - (pad[6:-4,0] - pad[2:-8,0])

    return feat
