This is SphinxTrain, Carnegie Mellon University's open source acoustic
model trainer. This directory contains the scripts and instructions
necessary for building models for the CMU Sphinx Recognizer.

This distribution is free software, see COPYING for licence.

For up-to-date information, please see the web site at

   http://cmusphinx.org

Among the interesting resources there, you will find a link to
"Resources to build a recognition system", with pointers to a
dictionary, audio data, acoustic model etc.

For the initial setup, please take a look at doc/tinydoc.txt.

These scripts and programs have been tested for building Sphinx-2,
Sphinx-3, and Sphinx-4 acoustic models.  Additional information can be
found in the web page above, and also at:

    http://www.speech.cs.cmu.edu/sphinxman/scriptman1.html

Looking at doc/tinydoc.txt will only make you *think* you know what's going
on.

Installation Guide:
^^^^^^^^^^^^^^^^^^^
This sections contain installation guide for various platforms. 
All Platforms:
^^^^^^^^^^^^^^
You will need Perl to use the scripts provided. Linux usually comes
with some version of Perl. If you do not have Perl installed, please
check:

http://www.perl.org

where you can download it for free. For Windows, a popular version,
ActivePerl, is available from ActiveState at:

http://www.activestate.com/Products/ActivePerl/

Linux/Unix Installation:
^^^^^^^^^^^^^^^^^^^^^^^^
This distribution now uses GNU autoconf to find out basic information
about your system, and should compile on most Unix and Unix-like
systems, and certainly on Linux.  To build, simply run

    ./configure
    make

(Or whatever you call GNU Make).  This should configure everything
automatically. The code has been tested with gcc.

Also, check the section title "All Platforms" above.

Windows Installation:
^^^^^^^^^^^^^^^^^^^^^
To compile SphinxTrain in Visual C++ 6.0, 
1, uncompress the file (e.g. using WinZip) and save it to the disk.
2, click SphinxTrain.dsw.
3, build all projects; each of them is a tool used by the training script.

Step 3 can be done easily in VC6.0 by clicking the "Build" menu at the
top bar, and selecting "Batch Build...". Make sure all projects are
selected, preferably the "Release" version of each. You can also run
with the "Debug" version, but this version is usually slower.

If you are using cygwin, the installation procedure is very similar to
the Unix installation. 

Also, check the section title "All Platforms" above.

Acknowldegments
^^^^^^^^^^^^^^^
This work was built over a large number of years at CMU by most of the
people in the Sphinx Group. Some code goes back to 1986. The most
recent work in tidying this up for release includes the following,
listed alphabetically (at least these are the people who are most
likely able to help you).



Alan W Black (awb@cs.cmu.edu)
Arthur Chan (archan@cs.cmu.edu)
Evandro Gouvea (egouvea+@cs.cmu.edu)
Ricky Houghton (ricky.houghton@cs.cmu.edu)
Kevin Lenzo (lenzo@cs.cmu.edu)
Ravi Mosur
Rita Singh (rsingh+@cs.cmu.edu)
Eric Thayer
