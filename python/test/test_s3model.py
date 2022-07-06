#!/usr/bin/env python

import unittest
from cmusphinx import s3model
from cmusphinx import s2mfc
from cmusphinx.feat import _1s_c_d_dd
import os


class TestS3Model(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'data', 'tidigits')
        self.acmod = s3model.S3Model(self.testdir)

    def test_created(self):
        self.assertTrue(abs(self.acmod.tmat[0][0,1] - 0.3326) < 0.01)
        self.assertTrue(abs(sum(self.acmod.mixw[0,0]) - 1.0) < 0.01)
        self.assertTrue(abs(self.acmod.var[0][0][0][0] - 0.2583) < 0.01)
        self.assertTrue(abs(self.acmod.var[35][0][0][38] - 16.9266) < 0.01)

    def test_compute(self):
        mfcc = s2mfc.open(os.path.join(self.testdir, 'man.ah.111a.mfc')).getall()
        mfcc -= mfcc.mean(0)
        feat = _1s_c_d_dd.compute(mfcc)
        senones = [0, 69, 135, 525]
        expected = [3.03518949e-36, 1.00000000e+00, 4.47046728e-16, 1.07179724e-01]
        senscr = self.acmod.senone_compute(senones, feat[0])
        for i,s in enumerate(senones):
            self.assertTrue(abs(senscr[s] - expected[i]) < 0.01)

if __name__ == '__main__':
    unittest.main()
