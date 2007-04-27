# Copyright (c) 2007 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Corpus classes for acoustic model training.

This module provides classes for representing a corpus of utterances
for acoustic modeling.  The Corpus class implements the iterator
protocol, acting as a list of Utterance objects.
"""
__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

import os

class Resource(object):
    """Resource associated with an utterance in a speech corpus.

    Any utterance has an arbitrary set of resources associated with
    it.  These are things such as waveforms, acoustic feature files,
    transcriptions and other forms of supervision, etc.
    """
    pass


class CtlEntry(object):
    """Entry in a control file"""
    def __init__(self, str):
        fields = str.split()
        if len(fields) == 4:
            self.fileid, self.sf, self.ef, self.uttid = fields
            self.sf = int(self.sf)
            self.ef = int(self.ef)
        else:
            self.fileid = self.uttid = str
            self.sf = 0
            self.ef = -1


class FileResource(Resource):
    def __init__(self, ctl_file, base_dir, file_ext, data_type=None):
        """Initialize a file-based resource.  The ctl_file argument is
        a list or other sequence of utterance IDs.  The base_dir and
        file_ext arguments describe where to locate resources in the
        filesystem.  The data_type argument specifies a class which is
        used to load and represent the resource (otherwise its
        filesystem path will simply be returned as a string.)"""
        self.ctl = iter(ctl_file)
        self.base_dir = base_dir
        self.file_ext = file_ext
        self.data_type = data_type

    def __iter__(self):
        return self

    def next(self):
        # This will raise StopIteration for us at EOF
        entry = CtlEntry(self.ctl.next().rstrip())
        path = os.path.join(self.base_dir, entry.fileid + self.file_ext)
        if self.data_type:
            return self.data_type(path)
        else:
            return path


class ListResource(Resource):
    def __init__(self, file_name, data_type=None):
        """Initialize a listing-based resource.  The file_name
        argument specifies the name of the listing file.  The
        data_type argument, which is optional, specifies a class which
        is used to represent each line in the file (otherwise it will
        simply be returned as a string.)"""
        self.data_type = data_type
        self.fh = open(file_name)

    def __iter__(self):
        return self

    def next(self):
        spam = self.fh.readline()
        if spam == "":
            raise StopIteration
        if self.data_type:
            return self.data_type(spam.rstrip())
        else:
            return spam.rstrip()


class Corpus(object):
    """Corpus of speech data."""
    def __init__(self, ctl_file):
        self.ctl = open(ctl_file)
        self.resources = []

    def __iter__(self):
        return self

    def next(self):
        utt = {}
        for k,v in self.resources.iteritems():
            utt[k] = v.next()

    def add_resource(self, name, res):
        self.resources[name] = res
