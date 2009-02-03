# Copyright (c) 2007 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
Sphinx-III format hypothesis segmentation files.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

def open(filename):
    """
    Open a Sphinx-III format hypothesis segmentation file for reading.
    @param filename Name of file to read
    @type filename string
    """
    return S3HypSeg(filename)

class S3HypSegEntry(object):
    """
    Entry in a Sphinx-III format hypothesis segmentation file.
    @ivar uttid: Utterance ID.
    @type uttid: string
    @ivar scale: Acoustic scaling factor.
    @type scale: int
    @ivar score: Viterbi path score.
    @type score: int
    @ivar ascr: Total acoustic score.
    @type ascr: int
    @ivar lscr: Total language model score.
    @type lscr: int
    @ivar segs: List of segmentations (name, start_frame, end_frame, ascr, lscr)
    @type segs: (string, int, int, int, int)
    """
    __fields__ = ['uttid', 'scale', 'score', 'ascr', 'lscr', 'segs']
    def __init__(self, line):
        fields = line.rstrip().split()
        # Strip S * T * A * L * sf
        self.uttid = fields[0]
        self.scale = int(fields[2])
        self.score = int(fields[4])
        self.ascr = int(fields[6])
        self.lscr = int(fields[8])
        fields[0:9] = ()
        sf = fields.pop(0)
        self.segs = []
        while fields:
            ascr, lscr, name, ef = fields[0:4]
            fields[0:4] = ()
            self.segs.append((name, int(sf), int(ef), int(ascr), int(lscr)))
            sf = ef

class S3HypSeg(file):
    """
    Class for reading Sphinx-III format hypothesis segmentation files.
    """
    def next(self):
        spam = self.readline()
        if spam:
            return S3HypSegEntry(spam)
        else:
            raise StopIteration
