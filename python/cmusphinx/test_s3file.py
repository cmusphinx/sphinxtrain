#!/usr/bin/env python

import unittest
from cmusphinx import s3file
import numpy
import os


class TestS3File(unittest.TestCase):
    def test_1dfile(self):
        data = numpy.array((1., 2., 3., 4.))
        with s3file.S3File_write("tmp.1d") as s3f:
            s3f.write1d(data)
        with s3file.S3File("tmp.1d") as s3f:
            data2 = s3f.read1d()
            # Assert that all elements are equal
            self.assertEqual(sum(data == data2), len(data))
        os.unlink("tmp.1d")

    def test_2dfile(self):
        data = numpy.array(((1., 2., 3., 4.), (5., 6., 7., 8.)))
        with s3file.S3File_write("tmp.2d") as s3f:
            s3f.write2d(data)
        with s3file.S3File("tmp.2d") as s3f:            
            data2 = s3f.read2d()
        # Assert that all elements are equal
        self.assertEqual((data == data2).sum(), data.size)
        os.unlink("tmp.2d")

    def test_3dfile(self):
        data = numpy.array((((1.,2.),(3.,4.)),((5.,6.),(7.,8.))))
        with s3file.S3File_write("tmp.3d") as s3f:
            s3f.write3d(data)
        with s3file.S3File("tmp.3d") as s3f:
            data2 = s3f.read3d()
        # Assert that all elements are equal
        self.assertEqual((data == data2).sum(), data.size)
        os.unlink("tmp.3d")


if __name__ == '__main__':
    unittest.main()
