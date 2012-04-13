#!/usr/bin/env python

"""
Adapt acoustic models using constrained maximum-likelihood linear regression.

This module implements single-class mean and variance adaptation using
CMLLR as described in M.J.F. Gales,
"Maximum likehood Linear Transformations for HMM-Based Speech recognition",

update only CD senones (one class)

TODO: Multiple regression classes.
"""

# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>, Stephan Vanni <svanni@vecsys.fr>"
__version__ = "$Revision $"

import numpy as np

import sys
import s3gaucnt
import s3gau
import s3mdef
import getopt
import os.path
import math

def estimate_cmllr(stats, inmean, invar, mdef):
    #Ws list
    Ws = []
    # Mllr iteration
    niter = 0
    # for i in range(0, inmean.n_feat): => SV : always 0 for one stream
    i = 0
    ndim = inmean.veclen[i]
    # Init A and bias
    A = np.eye(ndim)
    bias = np.zeros(ndim)
    # Collection of G matrices
    G = np.zeros((ndim, ndim+1, ndim+1))
    # K matrix (for the single class and stream)
    K = np.zeros((ndim, ndim+1))
    # W vector
    W = np.zeros(ndim+1)
    # Cofactors vector
    cofact = np.zeros(ndim+1)
    # Beta
    B = 0
    print 'Get statistics & sum it'
    # CD only : just sum over all CD densities
    for j in range(mdef.n_ci_sen, inmean.n_mgau):
#        print 'state = %i' % j
        for k in range(0, inmean.density):            
            # Mean ( vector, dim : ndim )
            mean = inmean[j][i][k]
            # Inverse variance (diagonal)
            # ( vector, dim : ndim )
            invvar = invar[j][i][k]
            # invar[j][i][k] is a vector (diagonal) and not a NxN matrix
            # if not full cov
            if len(invvar.shape) > 1:
                invvar = np.diag(invvar)
            invvar = 1./invvar.clip(1e-5,np.inf) 
            # Sum of variance statistics ( i.e sum(L_m_r o o^T) ) 
            # ( vector, dim : ndim )
            obsvar = stats.var[j][i][k]
            # Sum of posteriors (i.e. sum_t L_m_r(t) )
            dnom = stats.dnom[j][i][k]
            # Extended sum of mean statistics (i.e. sum(L_m_r o) ) 
            # ( vector, dim : ndim+1 )
            obsmean  = stats.mean[j][i][k]
            xobsmean = np.concatenate((obsmean, (dnom,)))
            # G{l} = \sum_r \Sigma_{l}^{-1} * \sum_t L_m_r oe oe^T (outer)
            #      = ...                 * [\sum_t L_m_r  (\sum_t  L_m_r o)^T]
            #                              [\sum_t L_m_r o \sum_t L_m_r o o^T]
            SumT = obsvar
            SumT = np.concatenate((SumT, (obsmean.T,)),axis=0)
            SumT = np.c_[SumT, xobsmean]
            for l in range(0, ndim):
                G[l] += invvar[l] * SumT
                # K{l} = \sum_r \Sigma_{l}^{-1} * \Mean{l} * L_m_r oe
                #      = ...                               * [\sum_t L_m_r (\sum_t  L_m_r o)^T]
            K += np.outer(invvar * mean, xobsmean)
            #Sum for all gausians
            B += dnom
    # End of collecting stats

    Ginv = np.zeros((ndim+1, ndim+1))
    while (niter < 10):
        niter += 1
        for i in range(0, ndim):
            Ginv = np.linalg.inv(G[i])
            #Ginv = np.linalg.pinv(G[i],rcond=1.0e-6)
            # Init W for convergence of likehood
            iniW = np.concatenate((A[i,:], (bias[i],)))
            # Get extended cofactors
            cofact = get_cofact(A,i)
            # Get alpha
            alpha  = get_alpha(Ginv,K[i],B,cofact)
            print "alpha : %f" % alpha
            W = np.zeros(ndim+1)
            tvec = alpha * cofact + K[i]
            W = np.dot(Ginv,tvec)
            like_new = get_row_like(G[i],K[i],cofact,B,W)
            like_old = get_row_like(G[i],K[i],cofact,B,iniW)
            if (like_new > like_old) :
                A[i,:] = W[0:ndim]
                bias[i] = W[ndim]
            else:
                print 'NOT updating row %i, iter %i,( %f > %f )' % ( i , niter, like_old, like_new )
    #to preserve compatibility with write_mllr
    Wi = np.c_[bias,A]
    Ws.append(Wi)
    return Ws

def get_row_like(G,K,cofact,B,W):
    row_like = 0
    tvec = np.dot(W,G)
    row_like = np.dot(tvec - 2 * K, W)
    det = np.dot(cofact,W)
    row_like = ( math.log(math.fabs(det)) * B ) - ( row_like / 2 )
    return row_like

def get_alpha(Ginv,K,B,cofact):
    alpha = 0
    tvec = np.zeros(K.size)
    #p{i}.G{i}^-1
    #tvec = np.dot(cofact,Ginv) 
    tvec = np.dot(Ginv,cofact) 
    #a = p{i}.G{i}^-1.p{i}
    a = np.dot(tvec,cofact)
    #b = p{i}.G{i}^-1.k{i}
    b = np.dot(tvec,K)
    #c = -beta
    c = -B
    # discriminant
    d = b * b - 4 * a * c
    if ( d < 0 ) :
        #solutions must be real
        print 'Warning : determinant < 0'
        d = 0
    d = math.sqrt(d)
    alpha1 = (- b + d ) / ( 2 * a )
    alpha2 = (- b - d ) / ( 2 * a )
    like1 = get_alpha_like(a,b,c,alpha1)
    like2 = get_alpha_like(a,b,c,alpha2)
    if (like1 > like2) :
        alpha = alpha1
    else:
        alpha = alpha2
    return alpha

def get_alpha_like (a, b, c, alpha):
    return (-c * math.log( math.fabs(alpha * a + b) ) - ( alpha * alpha * a) / 2)

def get_cofact(A, i):
    # Cofactors are compute like this : 
    # Cofact(A) = det(A) * A^{-1}
    #
    # For determinant computing , slogdet is more suitable for large matrix but not available with numpy < 2.0
    #   (sign, logdet) = np.linalg.slogdet(A)
    #   det = sign * np.exp(logdet)
    det = np.linalg.det(A)
    # Invert of matrix A
    #ainv = np.linalg.pinv(A,rcond=1.0e-6)
    ainv = np.linalg.inv(A)
    # Only for i row
    # ainv = ainv[i,:]
    # Only for i columns
    ainv = ainv[:,i]
    cofact = det * ainv
    # cofact = ainv
    # Extend cofactors
    cofact = np.concatenate((cofact, (0,)))
    return cofact

def write_mllr(fout, Ws):
    """
    Write out MLLR transformations of the means in the format that
    Sphinx3 understands.

    @param Ws: MLLR transformations of means, one per feature stream
    @ptype Ws: list(numpy.ndarray)
    @param fout: Filename or filehandle to write to.
    @ptype fout: string or file
    """
    if isinstance(fout, file):
        fh = fout
    else:
        fh = file(fout, 'w')
    # One-class MLLR for now
    fh.write("%d\n" % 1)
    fh.write("%d\n" % len(Ws))
    for i,W in enumerate(Ws):
        fh.write("%d\n" % W.shape[0])
        # Write rotation and bias terms separately
        for w in W:
            for x in w[1:]:
                fh.write("%f " % x)
            fh.write("\n")
        for x in W[:,0]:
            fh.write("%f " % x)
        fh.write("\n")

def solve_transform(Ws):
    """
    Solve the CMLLR tranformations from A' and b' as follow :
    A'^-1   = A
    A'^-1b' = b
    ( p.6 , formulae (24),
      Gales97-mllr.pdf, 
      "Maximum likehood Linear Transformations for HMM-Based Speech recognition", M.J.F Gales
    )
    @param Ws: Derived transformations
    @ptype Ws: list(numpy.ndarray)
    @return: CMLLR tranformations
    @type  : list(numpy.ndarray)
    """
    Wp =[]
    #one single stream , Ws = W
    for i,W in enumerate(Ws):
        # Get rotation and bias terms separately
        b = W[:,0]
        A = W[:,1:]
    Ap = np.linalg.inv(A)
    #Ap = np.linalg.pinv(A,rcond=1.0e-6)
    bp = np.linalg.solve(A,b)
    #to preserve compatibility with write_mllr
    Wi = np.c_[bp,Ap]
    Wp.append(Wi)
    return Wp

def solve_mllr(Wp, inmean, invar, mdef) :
    """
    Solve he CMLLR tranformations for means
    new_mean = A'.old_mean - b' ( !! be aware of the minus !! )
    new_var  = A'.old_var.A'^T
    @param Wp: CMLLR transformations
    @ptype Wp: list(numpy.ndarray)
    @param inmean: Input mean parameters
    @type inmean: cmusphinx.s3gau.S3Gau
    @param invar: Input variance parameters
    @type invar: cmusphinx.s3gau.S3Gau
    @return: Tranformed mean and variance parameters
    @type  : list(numpy.ndarray)
    """
    P = []
    #one single stream , Wp = W
    for i,W in enumerate(Wp):
        # Get rotation and bias terms separately
        bp = W[:,0]
        Ap = W[:,1:]      
    #one stream
    i = 0
    outmean = np.zeros((inmean.n_mgau,1,inmean.density,bp.size))
    outvar  = np.zeros((inmean.n_mgau,1,inmean.density,bp.size))
    for j in range(0, inmean.n_mgau):
        # CD only
        if (j < mdef.n_ci_sen) :
            outmean[j][i] = inmean[j][i]
            outvar[j][i]  = invar[j][i]
            continue
        for k in range(0, inmean.density):
            # Mean ( vector, dim : ndim )
            mean = inmean[j][i][k]
            outmean[j][i][k] = np.dot(Ap,mean) - bp
            # Variance ( vector, dim : ndim )
            var  = np.diag(invar[j][i][k])
            tmp  = np.dot(Ap,var)
            tmp2 = np.dot(tmp,Ap.T)
            outvar[j][i][k] = np.diag(tmp2)
    P.append(outmean)
    P.append(outvar)
    return P

if __name__ == '__main__':
    def usage():
        sys.stderr.write("Usage: %s INMEAN INVAR MDEF ACCUMDIRS...\n" % sys.argv[0])

    try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["help"])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    if len(args) < 3:
        usage()
        sys.exit(2)
    ldafn = None
    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
    outmean = 'means.cmllr'
    outvar  = 'variances.cmllr'
    inmean = s3gau.open(args[0])
    invar = s3gau.open(args[1]) 
    mdef = s3mdef.open(args[2])
    accumdirs = args[3:]
    stats = s3gaucnt.accumdirs_full(accumdirs)
    Ws = estimate_cmllr(stats, inmean, invar, mdef)
    write_mllr(open("cmllr_matrix", "w"), Ws)
    Wp = solve_transform(Ws)
    param = solve_mllr(Wp, inmean, invar, mdef)
    om = s3gau.open(outmean,"wb")
    om.writeall(param[0])
    om = s3gau.open(outvar,"wb")
    om.writeall(param[1])
