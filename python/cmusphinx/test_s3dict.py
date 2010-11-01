#!/usr/bin/env python

import s3dict
import unittest
import os
import sys

class TestS3Dict(unittest.TestCase):
    def setUp(self):
        self.basedir = os.path.dirname(__file__)

    def testRead(self):
        foodict = s3dict.open(os.path.join(self.basedir, "test", "foo.dict"))
        self.assert_('AH' in foodict.phoneset)
        self.assertEquals(foodict.get_phones('A'), ['AH'])
        self.assertEquals(foodict.get_alt_phones('A', 2), ['EY'])
        self.assertEquals(foodict.get_phones('ZSWANG'), ['S', 'W', 'AE', 'NG'])
        try:
            foo = foodict.get_phones('QRXG')
            print foo
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
        self.assertEquals(foodict['A'], ['AH'])
        self.assertEquals(foodict['A',2], ['EY'])
        self.assertEquals(foodict['A(2)'], ['EY'])
        self.assertEquals(foodict['ZSWANG'], ['S', 'W', 'AE', 'NG'])

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
        self.assertEquals(mydict.get_phones('A'), ['AH'])
        self.assertEquals(mydict.get_alt_phones('A', 2), ['EY'])
        self.assertEquals(mydict.get_phones('ZSWANG'), ['S', 'W', 'AE', 'NG'])
        mydict.set_alt_phones('A', 2, ['AA'])
        self.assertEquals(mydict.get_alt_phones('A', 2), ['AA'])
        self.assert_('ZSWANG' in mydict)
        mydict.del_phones('ZSWANG')
        self.assert_('ZSWANG' not in mydict)
        self.assert_('NG' not in mydict.phoneset)

    def testUnion(self):
        foodict = s3dict.open(os.path.join(self.basedir, "test", "foo.dict"))
        bardict = s3dict.open(os.path.join(self.basedir, "test", "bar.dict"))
        bazdict = s3dict.union(foodict, bardict)
        self.assertEquals(foodict['ACTUALLY'], bazdict['ACTUALLY'])
        self.assert_('ABANDONED' in bazdict)
        self.assert_('ZONES' in bazdict)
        self.assert_('ZSWANG' in bazdict)

if __name__ == '__main__':
    unittest.main()
