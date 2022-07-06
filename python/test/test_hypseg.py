#!/usr/bin/env python

import unittest
from cmusphinx import hypseg
import os


class TestHypseg(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'data')

    def test_read(self):
        hseg = hypseg.open(os.path.join(self.testdir, "an4-1-1.matchseg"))
        for spam in hseg:
            self.assertTrue(spam.uttid.startswith("an4test"))
        with hypseg.open(os.path.join(self.testdir, "an4-1-1.matchseg")) as hseg:
            for spam in hseg:
                self.assertTrue(spam.uttid.startswith("an4test"))


if __name__ == '__main__':
    unittest.main()
