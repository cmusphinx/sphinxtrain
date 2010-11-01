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
from itertools import chain
import re

def open(file, *args, **kwargs):
    return S3Dict(file, *args, **kwargs)

altre = re.compile(r'(.*)\(([^\)]+)\)')
def word_to_str(w):
    if w[1] == 1:
        return w[0]
    else:
        return "%s(%d)" % w

def str_to_word(w):
    m = altre.match(w)
    if m:
        word, alt = m.groups()
        return word, int(alt)
    else:
        return w, 1

tagre = re.compile(r':[^\(]+')

class S3Dict(dict):
    """
    Class for reading / processing Sphinx format dictionary files.
    """
    def __init__(self, infile=None, preserve_alts=False, strip_classtags=False):
        self.preserve_alts = preserve_alts
        self.strip_classtags = strip_classtags
        self.phoneset = defaultdict(int)
        self.maxalt = defaultdict(int)
        if infile != None:
            self.read(infile)

    def __contains__(self, key):
        if not isinstance(key, tuple):
            word, alt = str_to_word(key)
            return dict.__contains__(self, (word, alt))
        else:
            return dict.__contains__(self, key)

    def __getitem__(self, key):
        if not isinstance(key, tuple):
            word, alt = str_to_word(key)
            return self.get_alt_phones(word, alt)
        else:
            return self.get_alt_phones(*key)

    def __setitem__(self, key, val):
        if not isinstance(key, tuple):
            word, alt = str_to_word(key)
            return self.set_alt_phones(word, alt, val)
        else:
            w, p = key
            self.set_alt_phones(w, p, val)

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
            if self.strip_classtags:
                word = self.tagre.sub("", word)
            phones = spam[1:]
            self[word] = phones
                
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
            word = word_to_str(k)
            fh.write("%s\t%s\n" % (word.encode('utf8'), " ".join(self[k])))

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
        if not dict.__contains__(self, (word, 1)):
            raise KeyError
        elif not dict.__contains__(self, (word, alt)):
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
        self.maxalt[word] += 1
        dict.__setitem__(self, (word, self.maxalt[word]), phones)
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
        if not dict.__contains__(self, (word, alt)):
            raise IndexError, "Alternate pronunciation index %d does not exist" % alt
        for ph in self[word, alt]:
            self.phoneset[ph] -= 1
            if self.phoneset[ph] == 0: # FIXME: make a class
                del self.phoneset[ph]
        del self[word, alt]
        if alt == self.maxalt[word]:
            self.maxalt[word] -= 1
        if not self.preserve_alts:
            alts = []
            for i in range(1, self.maxalt[word] + 1):
                if (word,i) in self:
                    alts.append(dict.__getitem__(self, (word, i)))
                    del self[word, i]
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
            if dict.__contains__(self, (word, i)):
                for ph in self[word,i]:
                    self.phoneset[ph] -= 1
                    if self.phoneset[ph] == 0: # FIXME: make a class
                        del self.phoneset[ph]
                dict.__delitem__(self, (word, i))
        del self.maxalt[word]

    def words(self):
        """
        Iterate over base words in this dictionary.
        """
        for word,alt in self:
            if alt == 1:
                yield word

    def alts(self, word):
        """
        Iterate over alternative pronunciations for a word.
        """
        for i in range(1, self.maxalt[word] + 1):
            if (word,i) in self:
                yield self[word,i]

def copy(self, other, w):
    """
    Copy all pronunciations of w from other to self.
    """
    for phones in other.alts(w):
        self.add_alt_phones(w, phones)

def union(self, other):
    """
    Compute the union of two dictionaries, returning
    the resulting dictionary.

    Lists of alternate pronunciations for words will be merged between
    the two dictionaries.  The numeric identifiers of said alternates
    will not be preserved, however, the default pronunciation in the
    output is guaranteed to be the default pronunciation in self.
    """
    
    newdict = self.__class__()
    sw = set(self.words())
    ow = set(other.words())
    # Simply copy words not shared between inputs
    for w in sw ^ ow:
        if w in self:
            copy(newdict, self, w)
        elif w in other:
            copy(newdict, other, w)
    # Merge pronunciations for all others
    for w in sw & ow:
        # Uniquify them
        prons = set()
        for phones in chain(self.alts(w), other.alts(w)):
            prons.add(tuple(phones))
        # Set default pronunciation
        newdict[w] = self[w]
        prons.remove(tuple(self[w]))
        # Add all others in arbitrary order
        for phones in prons:
            newdict.add_alt_phones(w, list(phones))
    return newdict

def convert_to_40(d):
    badalts = []
    for w in d.words():
        baw = []
        for i, p in enumerate(d.alts(w)):
            # Remove 'DX' outright because it is ambiguous
            if 'DX' in p:
                baw.append((w, i+1))
            # Otherwise do AX -> AH, AXR -> ER
            else:
                j = 0
                while j < len(p):
                    if p[j] == 'AX':
                        p[j] = 'AH'
                    elif p[j] == 'IX':
                        p[j] = 'IH'
                    elif p[j] == 'AXR':
                        p[j] = 'ER'
                    elif p[j] == 'NX':
                        p[j] = 'N'
                    elif p[j] == 'TD':
                        p[j] = 'T'
                    elif p[j] == 'DD':
                        p[j] = 'D'
                    elif p[j] == 'TS':
                        p[j:j+1] = ['T', 'S']
                        j += 1
                    elif p[j] == 'EN':
                        p[j:j+1] = ['AH', 'N']
                        j += 1
                    elif p[j] == 'EM':
                        p[j:j+1] = ['AH', 'M']
                        j += 1
                    j += 1
        badalts.extend(baw[::-1])
    for w, i in badalts:
        d.del_alt_phones(w, i)    
