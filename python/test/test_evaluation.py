#!/usr/bin/env python

from cmusphinx import evaluation
import unittest


class TestEvaluation(unittest.TestCase):
    def test_parse_hyp(self):
        t, u, s = evaluation.parse_hyp("FOO BAR BLATZ (ught)")
        self.assertEqual(t.strip(), "FOO BAR BLATZ")
        self.assertEqual(u, "ught")
        self.assertEqual(s, 0)
        t, u, s = evaluation.parse_hyp("   <s> FOOY BAR HURRRF </s>  (ught -2342)")
        self.assertEqual(t.strip(), "FOOY BAR HURRRF")
        self.assertEqual(u, "ught")
        self.assertEqual(s, -2342)
        t, u, s = evaluation.parse_hyp("HEYYYY")
        self.assertEqual(t, None)
        self.assertEqual(u, None)
        self.assertEqual(s, 0)

        
if __name__ == "__main__":
    unittest.main()
