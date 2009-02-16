# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read/write Sphinx-III LDA/MLLT/covariance files.

This module reads and writes the linear transformation files used by
Sphinx-III.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from s3file import S3File, S3File_write

class S3LDAFile(S3File):
    "Read Sphinx-III format LDA files"
    def __init__(self, file, mode):
        S3File.__init__(self, file, mode)
        self._params = self._load()

    def readgauheader(self):
        if self.fileattr["version"] != "0.1":
            raise Exception("Version mismatch: must be 0.1 but is "
                            + self.fileattr["version"])

    def _load(self):
        self.readgauheader()
        self.fh.seek(self.data_start, 0)
        return self.read3d()

class S3LDAFile_write(S3File_write):
    "Write Sphinx-III format LDA files"
    def __init__(self, filename=None, mode="wb", attr={"version":0.1}):
        S3File_write.__init__(self, filename, mode, attr)

    def writeall(self, stuff):
        self.write3d(stuff)

def open(filename, mode="rb"):
    if mode in ("r", "rb"):
        return S3LDAFile(filename, mode)
    elif mode in ("w", "wb"):
        return S3LDAFile_write(filename, mode)
    else:
        raise Exception, "mode must be 'r', 'rb', 'w', or 'wb'"

