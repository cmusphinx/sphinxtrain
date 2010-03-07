#!/usr/bin/env python

import s3dict
import unittest
import os
import sys

class TestS3Dict(unittest.TestCase):
    def setUp(self):
        self.basedir = os.path.dirname(__file__)

    def testCreate(self):
        foodict = s3dict.open(os.path.join(self.basedir, "foo.dict"))


if __name__ == '__main__':
    unittest.main()
