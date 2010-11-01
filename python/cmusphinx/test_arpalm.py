#!/usr/bin/env python

import arpalm
import unittest
import os
import math

LOG10TOLOG = math.log(10)

class TestArpaLM(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'test')

    def test_load(self):
        lm = arpalm.ArpaLM(os.path.join(self.testdir, "100.arpa.gz"))
        score = lm.score('addition')
        self.assert_(abs(score - (-3.2721 * LOG10TOLOG)) < 0.01)
        score = lm.score('vector', 'variance', 'zero')
        self.assert_(abs(score - (-0.9404 * LOG10TOLOG)) < 0.01)
        # Backoff with backoff weight
        score = lm.score('parameters', 'variance', 'zero')
        self.assert_(abs(score - ((-1.3188 + -0.8170) * LOG10TOLOG)) < 0.01)
        # Backoff without backoff weight
        score = lm.score('vector', 'variance', 'abnormal')
        self.assert_(abs(score - (-0.8378  * LOG10TOLOG)) < 0.01)

if __name__ == '__main__':
    unittest.main()
