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

import re

def open(file):
    return S3Dict(file)

class S3Dict(dict):
    """
    Class for reading / processing Sphinx format dictionary files.
    """
    def __init__(self, infile=None):
        self.phoneset = set()
        if infile != None:
            self.read(infile)

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
            word = spam[0]
            phones = spam[1:]
            m = self.altre.match(word)
            if m:
                word, alt = m.groups()
                self[word,int(alt)] = phones
            else:
                self[word,1] = phones
            for ph in phones:
                self.phoneset.add(ph)
                
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

    def __getitem__(self, key):
        if not isinstance(key, tuple):
            m = self.altre.match(key)
            if m:
                word, alt = m.groups()
                return self.get((word,int(alt)))
            else:
                return self.get((key,1))
        else:
            return self.get(key)

