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

class FileResourceIterator(object):
    """
    Iterator over items in a FileResource.
    """
    def __init__(self, resource):
        self.res = resource
        self.ctl = iter(resource.ctl_file)

    def next(self):
        # This will raise StopIteration for us at EOF
        entry = self.ctl.next()
        if isinstance(entry, CtlEntry):
            path = os.path.join(self.res.base_dir, entry.fileid + self.res.file_ext)
        else:
            path = os.path.join(self.res.base_dir, entry + self.res.file_ext)
        if self.res.data_type:
            return self.res.data_type(path)
        else:
            return path

class FileResource(Resource):
    def __init__(self, ctl_file, base_dir, file_ext, data_type=None):
        """
        Initialize a file-based resource.
        @param ctl_file: Control file resource on which this is based
        @ptype ctl_file: iterator(CtlEntry)
        @param base_dir: Base directory to prepend to control entries
        @param file_ext: Filename extension to append to control entries
        @param data_type: Class to construct from entries.
        @ptype data_type: type
        """
        self.ctl_file = ctl_file
        self.base_dir = base_dir
        self.file_ext = file_ext
        self.data_type = data_type

    def __iter__(self):
        return FileResourceIterator(self)

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

class ListResourceIterator(object):
    """
    Iterator over items in a ListResource.
    """
    def __init__(self, resource):
        self.fh = open(resource.file_name)
        self.data_type = resource.data_type

    def next(self):
        spam = self.fh.readline()
        if spam == "":
            raise StopIteration
        if self.data_type:
            return self.data_type(spam.rstrip())
        else:
            return spam.rstrip()

class ListResource(Resource):
    """
    Corpus resource consisting of lines in a text file, of some data
    type.  This includes things like control and transcript files.
    """
    def __init__(self, file_name, data_type=None):
        """
        Initialize a listing-based resource.

        If no data_type argument is specified, each item in the list
        will be returned as a string.
        
        @param file_name: File to read resource from
        @ptype file_name: string
        @param data_type: Class implementing the data type of each item
        @ptype data_type: type
        """
        self.data_type = data_type
        self.file_name = file_name

    def __iter__(self):
        return ListResourceIterator(self)

class CorpusIterator(object):
    """
    Iterator over elements in a Corpus.
    """
    def __init__(self, corpus, part=1, npart=1):
        self.corpus = corpus
        self.iters = {}
        if npart > 1:
            pass
        else:
            for k, v in corpus.resources.iteritems():
                self.iters[k] = iter(v)

    def next(self):
        utt = {}
        for k,v in self.iters.iteritems():
            utt[k] = v.next()
        return utt

class Corpus(object):
    """Corpus of speech data."""
    def __init__(self, ctl_file):
        self.ctl = ListResource(ctl_file, CtlEntry)
        self.resources = { 'ctl' : self.ctl }

    def __iter__(self):
        return CorpusIterator(self)

    def add_resource(self, name, res):
        self.resources[name] = res
