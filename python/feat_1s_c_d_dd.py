# feat_1s_c_d_dd.py: Compute Sphinx-III 1s_c_d_dd features
#
# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III
#
# Author: David Huggins-Daines

from Numeric import shape, zeros, concatenate

def compute(mfcc):
    """Compute 39-dimensional single-stream features consisting of cepstra
     and their first and second time derivatives."""
    r, c = shape(mfcc)

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

