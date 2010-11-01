#!/usr/bin/env python

import hmm
import unittest
import s3model
import s2mfc
from feat import _1s_c_d_dd
import os
from math import log

class TestHMM(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'test', 'tidigits')
        self.acmod = s3model.S3Model(self.testdir)

    def test_create(self):
        h1 = hmm.HMM(self.acmod.mdef.pid2sseq(352),
                     self.acmod.tmat[self.acmod.mdef.pid2tmat(352)])

    def test_forward_backward(self):
        pid = self.acmod.mdef.phone_id('SIL')
        h1 = hmm.HMM(self.acmod.mdef.pid2sseq(pid),
                     self.acmod.tmat[self.acmod.mdef.pid2tmat(pid)])
        mfcc = s2mfc.open(os.path.join(self.testdir, 'man.ah.111a.mfc')).getall()
        mfcc -= mfcc.mean(0)
        feat = _1s_c_d_dd.compute(mfcc)
        alpha = None
        self.alpha = []
        for f in feat[0:50]:
            senscr = self.acmod.senone_compute(h1.iter_senones(), f)
            alpha = hmm.forward_evaluate(h1, senscr, alpha)
            self.alpha.append(alpha)
        beta = None
        self.beta = []
        for f in feat[50:0:-1]: # Note that this is time-shifted by
                                # one from the forward pass above
            senscr = self.acmod.senone_compute(h1.iter_senones(), f)
            beta = hmm.backward_evaluate(h1, senscr, beta)
            self.beta.append(beta)
        self.beta.reverse()
        ll = 0
        for a,b in zip(self.alpha, self.beta):
            newll = sum(a*b)
            if ll != 0:
                self.assert_(abs(log(ll) - log(newll)) < 0.1)
            ll = newll

class TestPhoneHMM(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'test', 'tidigits')
        self.acmod = s3model.S3Model(self.testdir)
        self.factory = hmm.PhoneHMMFactory(self.acmod)

    def test_create(self):
        pid = self.acmod.mdef.phone_id('OW_four', 'F_four', 'R_four')
        h1 = hmm.HMM(self.acmod.mdef.pid2sseq(pid),
                     self.acmod.tmat[self.acmod.mdef.pid2tmat(pid)])
        h2 = self.factory.create('OW_four', 'F_four', 'R_four')
        self.assertEquals(h1[0], h2[0])

class TestHMMGraph(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'test', 'tidigits')
        self.acmod = s3model.S3Model(self.testdir)
        self.factory = hmm.PhoneHMMFactory(self.acmod)

    def test_create(self):
        h1 = self.factory.create('F_four', 'SIL', 'OW_four')
        h2 = self.factory.create('OW_four', 'F_four', 'R_four')
        h3 = self.factory.create('R_four', 'OW_four', 'SIL')
        hg = hmm.HMMGraph(h1, h2, h3)
        self.assertEquals(hg[0], h1[0])
        self.assertEquals(hg[4], h2[0])
        self.assertEquals(hg[8], h3[0])

    def test_forward_backward(self):
        mfcc = s2mfc.open(os.path.join(self.testdir, 'man.ah.111a.mfc')).getall()
        mfcc -= mfcc.mean(0)
        feat = _1s_c_d_dd.compute(mfcc)
        hg = hmm.HMMGraph(self.factory.create('SIL'),
                          self.factory.create('SIL'))
        alpha = None
        self.alpha = []
        for f in feat[0:15]:
            senscr = self.acmod.senone_compute(hg.iter_senones(), f)
            alpha = hmm.forward_evaluate(hg, senscr, alpha)
            self.alpha.append(alpha)
        beta = None
        self.beta = []
        for f in feat[15:0:-1]: # Note that this is time-shifted by
                                # one from the forward pass above
            senscr = self.acmod.senone_compute(hg.iter_senones(), f)
            beta = hmm.backward_evaluate(hg, senscr, beta)
            self.beta.append(beta)
        self.beta.reverse()
        ll = 0
        for a,b in zip(self.alpha, self.beta):
            newll = sum(a*b)
            if ll != 0:
                self.assert_(abs(log(ll) - log(newll)) < 0.1)
            ll = newll

def read_dict(dictfile):
    fh = open(dictfile)
    out = {}
    for spam in fh:
        eggs = spam.rstrip().split()
        word = eggs[0]
        phones = eggs[1:]
        out[word] = phones
    return out

class TestSentenceHMM(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'test', 'tidigits')
        acmod = s3model.new(self.testdir)
        dictionary = read_dict(os.path.join(self.testdir, 'dictionary'))
        self.factory = hmm.SentenceHMMFactory(acmod, dictionary)

if __name__ == '__main__':
    unittest.main()

