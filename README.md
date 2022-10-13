SphinxTrain 5.0.0
=================

This is SphinxTrain, Carnegie Mellon University's open source acoustic
model trainer. This directory contains the scripts and instructions
necessary for building models for the CMU Sphinx Recognizer.

This distribution is free software, see LICENSE for licence.

For up-to-date information, please see the web site at

   https://cmusphinx.github.io

Among the interesting resources there, you will find a link to
"Resources to build a recognition system", with pointers to a
dictionary, audio data, acoustic model etc.

For introduction in training the acoustic model see the tutorial

https://cmusphinx.github.io/wiki/tutorialam

Installation Guide:
-------------------

This sections contain installation guide for various platforms. 

All Platforms:
--------------

You will unfortunately need both Perl and Python to use the scripts
provided. Linux usually comes with some version of Perl and Python. If
you do not have Perl installed, please check:

http://www.perl.org

where you can download it for free. For Windows, if you insist on not
using Windows Subsystem for Linux, a popular version, ActivePerl, is
available from ActiveState at:

https://www.activestate.com/products/perl/

Python for Windows can be obtained from:

http://www.python.org/download/

For some advanced techniques (which are not enabled by default) you
will need NumPy and SciPy.  Packages for NumPy and SciPy can be
obtained from:

http://scipy.org/Download

Or you can use Anaconda which makes all of this somewhat easier:

https://www.anaconda.com/products/distribution

If you wish to use the grapheme-to-phoneme support, you will need
rather specific versions of
[OpenFST](https://www.openfst.org/twiki/bin/view/FST/WebHome) and
[OpenGRM
NGram](https://www.opengrm.org/twiki/bin/view/GRM/NGramLibrary).  It
is known to work with OpenFST 1.6.3, and known *not* to work with
1.8.2. There is probably nothing you want in the latest version
anyway, and compiling it will consume several hours of your life and
several gigabytes of your disk for no good reason, so best to just use
what Ubuntu 20.04 LTS or 22.04 LTS will install for you with:

    apt install libfst-dev libngram-dev

See the note about `-DBUILD_G2P=ON` below to enable G2P support.

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

This should configure everything automatically. The code has been
tested with gcc.

To enable G2P, you need to add a magic incantation to the first
command above, namely:

    cmake -S . -B build -DBUILD_G2P=ON
    
You can also enable shared libraries with `-DBUILD_SHARED_LIBS=ON`,
but I suggest that you *not* do that unless you have a very good
reason.

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
