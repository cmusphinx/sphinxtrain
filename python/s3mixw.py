# s3mixw.py: Read/write Sphinx-III mixture weight files
#
# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III
#
# Author: David Huggins-Daines

import s3file

def open(filename, mode="rb"):
    if mode in ("r", "rb"):
        return S3MixwFile(filename, mode)
    elif mode in ("w", "wb"):
        return S3MixwFile_write(filename, mode)
    else:
        raise Exception, "mode must be 'r', 'rb', 'w', or 'wb'"

class S3MixwFile(s3file.S3File):
    "Read Sphinx-III format mixture weight files"

    def getall(self):
        try:
            return self._params
        except AttributeError:
            self._params = self._load()
            return self._params

    def readgauheader(self):
        if self.fileattr["version"] != "1.0":
            raise Exception("Version mismatch: must be 1.0 but is "
                            + self.fileattr["version"])

    def _load(self):
        self.readgauheader()
        self.fh.seek(self.data_start, 0)
        return self.read3d()

class S3MixwFile_write(s3file.S3File_write):
    "Write Sphinx-III format mixture weight files"

    def writeall(self, stuff):
        self.write3d(stuff)
