#!/usr/bin/env python

import arpalm
import unittest
import os
import sys
import math

LOG10TOLOG = math.log(10)

class TestArpaLM(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(sys.argv[0])
        self.testdir = os.path.join(thisdir, 'test')

    def test_load(self):
        lm = arpalm.ArpaLM(os.path.join(self.testdir, "100.arpa.gz"))
        self.assert_(abs(lm.score('addition') - (-3.2721 * LOG10TOLOG)) < 0.01)
        self.assert_(abs(lm.score('zero', 'variance', 'vector')
                         - (-0.9404 * LOG10TOLOG)) < 0.01)
        # Backoff with backoff weight
        self.assert_(abs(lm.score('zero', 'variance', 'parameters')
                         - ((-1.3188 + -0.8170) * LOG10TOLOG)) < 0.01)
        # Backoff without backoff weight
        self.assert_(abs(lm.score('abnormal', 'variance', 'vector')
                         - (-0.8378  * LOG10TOLOG)) < 0.01)

if __name__ == '__main__':
    unittest.main()
