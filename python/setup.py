#!/usr/bin/env python

from distutils.core import setup

setup(name='SphinxTrain',
      version='0.1',
      description='SphinxTrain Python modules',
      author='David Huggins-Daines',
      author_email='dhuggins@cs.cmu.edu',
      url='http://www.cmusphinx.org/',
      packages=['sphinx', 'sphinx.feat'],
      requires=['numpy', 'scipy'],
      )
