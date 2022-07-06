CMU Sphinx Python Modules
=========================

These are a number of modules that can be used to do speech
processing, generally oriented towards CMU Sphinx file formats.  In
fact, there is a wide variety of stuff here, mostly things created
during my (David Huggins-Daines) PhD work.  You will find for instance
pure-Python (but using numpy and scipy) implementations of certain
feature extraction and adaptation algorithms, along with code to read
and write the various files produced by SphinxTrain.

There is some code to convert things to FSTs and play with them, but
it does not work, since the Python bindings for OpenFst have mutated
greatly since it was written.  In fact, it may have used my own
personal wrapper library which may or may not still exist.  Sorry.
