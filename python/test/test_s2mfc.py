#!/usr/bin/env python

import unittest
import tempfile
from cmusphinx import s2mfc
import os


class TestS2MFC(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'data')

    def test_mfcfile(self):
        with s2mfc.open(os.path.join(self.testdir, "sa1.mfc")) as s2f:
            mfcc = s2f.getall()
            self.assertAlmostEqual(mfcc.mean(0)[0], 7.58447343)
        with tempfile.NamedTemporaryFile() as nt:
            with s2mfc.open(nt.name, "w") as s2f:
                s2f.writeall(mfcc)
            with s2mfc.open(nt.name) as s2f:
                mfcc = s2f.getall()
            self.assertAlmostEqual(mfcc.mean(0)[0], 7.58447343)


if __name__ == '__main__':
    unittest.main()
