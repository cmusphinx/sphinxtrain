#!/usr/bin/env python

import unittest
from cmusphinx import s3gaucnt, s3mixw
import os


class TestS3GauCnt(unittest.TestCase):
    def setUp(self):
        thisdir = os.path.dirname(__file__)
        self.testdir = os.path.join(thisdir, 'data', 'bwaccumdir')

    def test_read_dirs(self):
        gauden_all = s3gaucnt.accumdirs([os.path.join(self.testdir, d)
                                         for d in os.listdir(self.testdir)])
        gauden1 = s3gaucnt.open(os.path.join(self.testdir,
                                             "an4_buff_1",
                                             "gauden_counts"),
                                "rb")
        gauden2 = s3gaucnt.open(os.path.join(self.testdir,
                                             "an4_buff_2",
                                             "gauden_counts"),
                                "rb")
        self.assertEqual(gauden_all.mean[0][0][0, 0],
                         gauden1.mean[0][0][0, 0]
                         + gauden2.mean[0][0][0, 0])

    def test_read_mixw(self):
        gauden_all = s3gaucnt.accumdirs([os.path.join(self.testdir, d)
                                         for d in os.listdir(self.testdir)])
        mixw = s3mixw.accumdirs([os.path.join(self.testdir, d)
                                 for d in os.listdir(self.testdir)])
        self.assertEqual(gauden_all.dnom[0][0],
                         mixw.getall()[0][0, 0])


if __name__ == '__main__':
    unittest.main()
