#!/usr/bin/env python

import evaluation
import unittest
import os
import sys

class TestEvaluation(unittest.TestCase):
    def test_parse_hyp(self):
        t, u, s = evaluation.parse_hyp("FOO BAR BLATZ (ught)")
        self.assertEquals(t.strip(), "FOO BAR BLATZ")
        self.assertEquals(u, "ught")
        self.assertEquals(s, 0)
        t, u, s = evaluation.parse_hyp("   <s> FOOY BAR HURRRF </s>  (ught -2342)")
        self.assertEquals(t.strip(), "FOOY BAR HURRRF")
        self.assertEquals(u, "ught")
        self.assertEquals(s, -2342)
        t, u, s = evaluation.parse_hyp("HEYYYY")
        self.assertEquals(t, None)
        self.assertEquals(u, None)
        self.assertEquals(s, 0)
        
if __name__ == "__main__":
    unittest.main()
