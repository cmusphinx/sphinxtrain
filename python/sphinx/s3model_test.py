#!/usr/bin/env python

import hmm
import unittest
import s3model
import s2mfc
from feat import _1s_c_d_dd
import os
import sys
import numpy

class TestS3Model(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(sys.argv[0])
        self.testdir = os.path.join(thisdir, 'test', 'tidigits')
        self.acmod = s3model.S3Model(self.testdir)

    def test_created(self):
        self.assert_(abs(self.acmod.tmat[0][0,1] - 0.3326) < 0.01)
        self.assert_(abs(sum(self.acmod.mixw[0,0]) - 1.0) < 0.01)
        self.assert_(abs(self.acmod.var[0][0][0][0] - 0.2583) < 0.01)
        self.assert_(abs(self.acmod.var[35][0][0][38] - 16.9266) < 0.01)

    def test_compute(self):
        mfcc = s2mfc.open(os.path.join(self.testdir, 'man.ah.111a.mfc')).getall()
        mfcc -= mfcc.mean(0)
        feat = _1s_c_d_dd.compute(mfcc)
        senscr = self.acmod.senone_compute([0, 69, 135, 525], feat[0])
        expected = numpy.array((-165.69276147, -83.90997198, -119.25384053, -86.14322017))
        self.assert_(abs(sum(senscr - expected)) < 0.01)

if __name__ == '__main__':
    unittest.main()
