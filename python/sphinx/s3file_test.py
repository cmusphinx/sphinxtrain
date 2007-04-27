#!/usr/bin/env python

import hmm
import unittest
import s3file
import numpy
import os

class TestS3File(unittest.TestCase):
    def test_1dfile(self):
        data = numpy.array((1., 2., 3., 4.))
        s3file.S3File_write("tmp.1d").write1d(data)
        data2 = s3file.S3File("tmp.1d").read1d()
        # Assert that all elements are equal
        self.assertEquals(sum(data == data2), len(data))
        os.unlink("tmp.1d")

    def test_2dfile(self):
        data = numpy.array(((1., 2., 3., 4.), (5., 6., 7., 8.)))
        s3file.S3File_write("tmp.2d").write2d(data)
        data2 = s3file.S3File("tmp.2d").read2d()
        # Assert that all elements are equal
        self.assertEquals((data == data2).sum(), data.size)
        os.unlink("tmp.2d")

    def test_3dfile(self):
        data = numpy.array((((1.,2.),(3.,4.)),((5.,6.),(7.,8.))))
        s3file.S3File_write("tmp.3d").write3d(data)
        data2 = s3file.S3File("tmp.3d").read3d()
        # Assert that all elements are equal
        self.assertEquals((data == data2).sum(), data.size)
        os.unlink("tmp.3d")

if __name__ == '__main__':
    unittest.main()
