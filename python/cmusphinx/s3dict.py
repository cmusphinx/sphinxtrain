# Copyright (c) 2009 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read/write Sphinx dictionary files

This module reads and writes the text format dictionary files used by
SphinxTrain, Sphinx-III, and PocketSphinx.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision $"

from collections import defaultdict
import re

def open(file):
    return S3Dict(file)

class S3Dict(dict):
    """
    Class for reading / processing Sphinx format dictionary files.
    """
    def __init__(self, infile=None, preserve_alts=False):
        self.preserve_alts = preserve_alts
        self.phoneset = defaultdict(int)
        self.maxalt = defaultdict(int)
        if infile != None:
            self.read(infile)

    def __getitem__(self, key):
        if not isinstance(key, tuple):
            m = self.altre.match(key)
            if m:
                word, alt = m.groups()
                return self.get_alt_phones(word, int(alt))
            else:
                return self.get_phones(key)
        else:
            return self.get_alt_phones(*key)

    def __putitem__(self, key, val):
        if not isinstance(key, tuple):
            m = self.altre.match(key)
            if m:
                word, alt = m.groups()
                return self.set_alt_phones(word, int(alt), val)
            else:
                self.set_phones(key, val)
        else:
            w, p = key
            self.set_alt_phones(w, p, val)

    altre = re.compile(r'(.*)\(([^\)]+)\)')
    def read(self, infile):
        """
        Read dictionary from a file.
        """
        if isinstance(infile, file):
            fh = infile
        else:
            fh = file(infile)
        for line in fh:
            line = line.strip()
            # Skip any comment lines as in cmudict source
            if line.startswith('##'):
                continue
            if line.startswith(';;'):
                continue
            spam = line.split()
            word = unicode(spam[0], 'utf8')
            phones = spam[1:]
            # Why can't we just say self[word] = phones?
            self.__putitem__(word, phones)
                
    def write(self, outfile):
        """
        Write dictionary to a file.
        """
        if isinstance(outfile, file):
            fh = outfile
        else:
            fh = file(outfile, 'w')
        wlist = self.keys()
        wlist.sort()
        for k in wlist:
            word, alt = k
            if alt != 1:
                word = "%s(%d)" % (word, alt)
            fh.write("%-30s %s\n" % (word, " ".join(self[k])))

    def get_phones(self, word):
        """
        Get default pronunciation for word.

        If word is not present, KeyError will be raised.
        """
        return dict.__getitem__(self, (word, 1))

    def get_alt_phones(self, word, alt):
        """
        Get alternate pronunciaition #alt for word.

        Alternate pronunciations are numbered from 1, where 1 is the
        default pronunciation.  If word is not present, KeyError will
        be raised.  If no alternate pronunciation alt exists,
        IndexError will be raised.
        """
        
        if (word, 1) not in self:
            raise KeyError
        elif (word,alt) not in self:
            raise IndexError, "Alternate pronunciation index %d does not exist" % alt
        return dict.__getitem__(self, (word, alt))

    def set_phones(self, word, phones):
        """
        Set default pronunciation for word.
        """
        dict.__setitem__(self, (word, 1), phones)
        self.maxalt[word] = 1
        for ph in phones:
            self.phoneset[ph] += 1 # FIXME: should make a class for this

    def set_alt_phones(self, word, alt, phones):
        """
        Set alternate pronunciaition #alt for word.

        If alt is greater than the maximum alternate pronunciation
        index plus one for this dictionary, IndexError will be raised.

        """
        if alt > self.maxalt[word] + 1:
            raise IndexError, "Alternate pronunciation index %d too high" % alt
        dict.__setitem__(self, (word, alt), phones)
        self.maxalt[word] = max(alt, self.maxalt[word])
        for ph in phones:
            self.phoneset[ph] += 1

    def add_alt_phones(self, word, phones):
        """
        Add a new alternate pronunciation for word.
        """
        dict.__setitem__(self, (word, self.maxalt[word] + 1), phones)
        self.maxalt[word] += 1
        for ph in phones:
            self.phoneset[ph] += 1

    def del_alt_phones(self, word, alt):
        """
        Delete alternate pronunciation alt for word.

        If no such alternate pronunciation exists, IndexError will be
        raised.  If this S3Dict was created with preserve_alts=True,
        the indices of remaining alternate pronunciations will be
        preserved (you can still this alternative index with
        set_alt_phones()).  Otherwise, the remaining alternate
        pronunciations will be renumbered accordingly.

        """
        if (word, alt) not in self:
            raise IndexError, "Alternate pronunciation index %d does not exist" % alt
        for ph in self[word, alt]:
            self.phoneset[ph] -= 1
            if self.phoneset[ph] == 0: # FIXME: make a class
                del self.phoneset[ph]
        del self[word, alt]
        if alt == self.maxalt[word]:
            self.maxalt[word] -= 1
        if not self.preserve_alts:
            alts = list(self.alts(word))
            self.del_phones(word)
            for i, phones in enumerate(alts):
                dict.__setitem__(self, (word, i + 1), phones)
            self.maxalt[word] = len(alts)

    def del_phones(self, word):
        """
        Delete all pronunciations for word.

        If you only wish to delete the default pronunciation (it is
        strongly suggested that you don't do this), use
        del_alt_phones(word, 1).
        """
        for i in range(1, self.maxalt[word] + 1):
            if (word,i) in self:
                for ph in self[word,i]:
                    self.phoneset[ph] -= 1
                    if self.phoneset[ph] == 0: # FIXME: make a class
                        del self.phoneset[ph]
                dict.__delitem__(self, (word, i))
        del self.maxalt[word]

    def alts(self, word):
        """
        Iterate over alternative pronunciations for a word.
        """
        for i in range(1, self.maxalt[word] + 1):
            if (word,i) in self:
                yield self[word,i]
