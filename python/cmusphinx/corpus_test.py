#!/usr/bin/env python

import corpus
import wave
import unittest
import os
import sys

class TestFileResource(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(sys.argv[0])
        self.testdir = os.path.join(thisdir, 'test')
        self.testfile = os.path.join(thisdir, 'test', 'test.ctl')

    def test_string(self):
        ctl = corpus.ListResource(self.testfile, corpus.CtlEntry)
        res = iter(corpus.FileResource(ctl, self.testdir, '.wav'))
        self.assertEqual(res.next(), os.path.join(self.testdir, 'sa1.wav'))

    def test_wave(self):
        ctl = corpus.ListResource(self.testfile, corpus.CtlEntry)
        res = iter(corpus.FileResource(ctl, self.testdir, '.wav', wave.Wave_read))
        wav = res.next()
        self.assertEqual(wav.getnframes(), 45261)

class TestListResource(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(sys.argv[0])
        self.testfile = os.path.join(thisdir, 'test', 'test.ctl')
        
    def test_string(self):
        res = iter(corpus.ListResource(self.testfile))
        self.assertEqual(res.next(), 'sa1 1 20 sa1_1_20')
        self.assertEqual(res.next(), 'sa1 25 35 sa1_25_35')

    def test_entry(self):
        res = iter(corpus.ListResource(self.testfile, corpus.CtlEntry))
        entry = res.next()
        self.assertEqual(entry.fileid, 'sa1')
        self.assertEqual(entry.sf, 1)
        self.assertEqual(entry.ef, 20)
        self.assertEqual(entry.uttid, 'sa1_1_20')
        entry = res.next()
        self.assertEqual(entry.fileid, 'sa1')
        self.assertEqual(entry.sf, 25)
        self.assertEqual(entry.ef, 35)
        self.assertEqual(entry.uttid, 'sa1_25_35')

if __name__ == '__main__':
    unittest.main()
