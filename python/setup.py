#!/usr/bin/env python

from distutils.core import setup, Extension

setup(name='SphinxTrain',
      version='1.0',
      description='SphinxTrain Python modules',
      author='David Huggins-Daines',
      author_email='dhuggins@cs.cmu.edu',
      url='http://www.cmusphinx.org/',
      packages=['cmusphinx', 'cmusphinx.feat'],
      requires=['numpy', 'scipy'],
      )
