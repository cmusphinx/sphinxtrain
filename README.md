SphinxTrain
===========

This is SphinxTrain, Carnegie Mellon University's open source acoustic
model trainer. This directory contains the scripts and instructions
necessary for building models for the CMU Sphinx Recognizer.

This distribution is free software, see LICENSE for licence.

For up-to-date information, please see the web site at

   http://cmusphinx.sourceforge.net

Among the interesting resources there, you will find a link to
"Resources to build a recognition system", with pointers to a
dictionary, audio data, acoustic model etc.

For introduction in training the acoustic model see the tutorial

http://cmusphinx.sourceforge.net/wiki/tutorialam

Installation Guide:
-------------------

This sections contain installation guide for various platforms. 

All Platforms:
--------------

You will need Perl and Python to use the scripts provided. Linux
usually comes with some version of Perl. If you do not have Perl
installed, please check:

http://www.perl.org

where you can download it for free. For Windows, a popular version,
ActivePerl, is available from ActiveState at:

http://www.activestate.com/Products/ActivePerl/

Python can be obtained from:

http://www.python.org/download/

For some advanced techniques (which are not enabled by default) you
will need NumPy and SciPy.  Packages for NumPy and SciPy can be
obtained from:

http://scipy.org/Download

Linux/Unix Installation:
------------------------

This distribution uses CMake to find out basic information about your
system, and should compile on most Unix and Unix-like systems, and
certainly on Linux.  On reasonable Linux distributions, a suitable
version of CMake (at least 3.14) can be installed with your package
manager, or may already be there if you have installed development
tools.

On certain unreasonable distributions that are far too often installed
on "enterprise" or "cloud" or HPC systems, the version of CMake is
incredibly ancient, and the package manager will not help you, so you
will have to install it manually, following the instructions at
https://cmake.org/download/

To build, simply run:

    cmake -S . -B build
    cmake --build build

This should configure everything automatically. The code has been tested with gcc.

You do not need to install SphinxTrain to run it, simply run
`scripts/sphinxtrain` from the source directory when initializing a
training directory.  Note that you do need to build and install
PocketSphinx for evaluation to work properly, however.

You can also install SphinxTrain system-wide if you so desire:

    sudo cmake --build build --target install

This will put various files in `/usr/local/lib`,
`/usr/local/libexec/sphinxbase` and `/usr/local/share/sphinxbase` and
create `/usr/local/bin/sphinxbase`.

Also, check the section title "All Platforms" above.

Windows Installation:
---------------------

You can build with Visual Studio Code using the C++ and CMake
extensions.  This will create all the binaries in `build\Debug` or
`build\Release` depending on the configuration you select.  As above,
you can run `python ..\sphinxtrain\scripts\sphinxtrain` (or whatever
the path is to `scripts\sphinxtrain` in your source directory) to set
up and run training.

Note that you will need to have Perl on your path, among other things,
and also, note that none of this has been tested, so we suggest you just
use [Windows Subsystem for
Linux](https://learn.microsoft.com/en-us/windows/wsl/install), which
is really a lot faster and easier to use than the native Windows
command-line.

If you are using Windows Subsystem for Linux, the installation
procedure is identical to the Unix installation.

Also, check the section title "All Platforms" above.

Acknowldegments
---------------

The development of this code has included support at different times
by various United States Government agencies, under different programs,
including the Defence Advanced Projects Agency (DARPA) and the
National Science Foundation (NSF). We are grateful for their support.

This work was built over a large number of years at CMU by most of the
people in the Sphinx Group. Some code goes back to 1986. The most
recent work in tidying this up for release includes the following,
listed alphabetically (at least these are the people who are most
likely able to help you).

- Alan W Black (awb@cs.cmu.edu)
- Arthur Chan (archan@cs.cmu.edu)
- Evandro Gouvea (egouvea+@cs.cmu.edu)
- Ricky Houghton (ricky.houghton@cs.cmu.edu)
- David Huggins-Daines (dhdaines@gmail.com)
- Kevin Lenzo (kevinlenzo@gmail.com)
- Ravi Mosur
- Long Qin (lqin@cs.cmu.edu)
- Rita Singh (rsingh+@cs.cmu.edu)
- Eric Thayer
