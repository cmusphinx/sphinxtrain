#!/usr/bin/env python

from cmusphinx import s3dict
import unittest
import os


class TestS3Dict(unittest.TestCase):
    def setUp(self):
        self.basedir = os.path.dirname(__file__)

    def testRead(self):
        foodict = s3dict.open(os.path.join(self.basedir, "data", "foo.dict"))
        self.assertTrue('AH' in foodict.phoneset)
        self.assertEqual(foodict.get_phones('A'), ['AH'])
        self.assertEqual(foodict.get_alt_phones('A', 2), ['EY'])
        self.assertEqual(foodict.get_phones('ZSWANG'), ['S', 'W', 'AE', 'NG'])
        try:
            foo = foodict.get_phones('QRXG')
            print(foo)
        except KeyError:
            pass # Expected fail
        else:
            self.fail()
        try:
            foo = foodict.get_alt_phones('A',3)
        except IndexError:
            pass # Expected fail
        else:
            self.fail()
        try:
            foo = foodict.get_alt_phones('!@#$!@',3)
        except KeyError:
            pass # Expected fail
        else:
            self.fail()
        self.assertEqual(foodict['A'], ['AH'])
        self.assertEqual(foodict['A',2], ['EY'])
        self.assertEqual(foodict['A(2)'], ['EY'])
        self.assertEqual(foodict['ZSWANG'], ['S', 'W', 'AE', 'NG'])

    def testCreate(self):
        mydict = s3dict.S3Dict()
        mydict.set_phones('A', ['AH'])
        mydict.add_alt_phones('A', ['EY'])
        mydict.set_phones('ZSWANG', ['S', 'W', 'AE', 'NG'])
        mydict.set_alt_phones('A', 2, ['EY'])
        try:
            mydict.set_alt_phones('A', 5, ['AX'])
        except IndexError:
            pass # Expected fail
        else:
            self.fail()
        self.assertEqual(mydict.get_phones('A'), ['AH'])
        self.assertEqual(mydict.get_alt_phones('A', 2), ['EY'])
        self.assertEqual(mydict.get_phones('ZSWANG'), ['S', 'W', 'AE', 'NG'])
        mydict.set_alt_phones('A', 2, ['AA'])
        self.assertEqual(mydict.get_alt_phones('A', 2), ['AA'])
        self.assertTrue('ZSWANG' in mydict)
        mydict.del_phones('ZSWANG')
        self.assertTrue('ZSWANG' not in mydict)
        self.assertTrue('NG' not in mydict.phoneset)

    def testUnion(self):
        foodict = s3dict.open(os.path.join(self.basedir, "data", "foo.dict"))
        bardict = s3dict.open(os.path.join(self.basedir, "data", "bar.dict"))
        bazdict = s3dict.union(foodict, bardict)
        self.assertEqual(foodict['ACTUALLY'], bazdict['ACTUALLY'])
        self.assertTrue('ABANDONED' in bazdict)
        self.assertTrue('ZONES' in bazdict)
        self.assertTrue('ZSWANG' in bazdict)

if __name__ == '__main__':
    unittest.main()
