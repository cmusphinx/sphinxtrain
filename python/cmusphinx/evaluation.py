# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
Evaluation tools for CMU Sphinx.

This module provides some convenience functions for dealing with the
output of the batch mode decoder and performing evaluation tasks.
"""

import re
import os

linere = re.compile(r"^\s*(?:<s>)?\s*([^(]+?)(?:</s>)?\s*(?:\(([^\)\s]+)\s*([^\)]+)?\))\s*?")
def parse_hyp(line):
    """
    Parse a hypothesis file string into text, utterance ID, and
    optional Viterbi score.

    @param line: Input line
    @type line: str
    @return: (text, uttid, score)
    @rtype: (str, str, int)
    """
    m = linere.match(line)
    if m:
        text, uttid, score = m.groups()
        if score == None:
            score = 0
        else:
            score = int(score)
        return text, uttid, score
    else:
        return (None, None, 0)

def hypfile(infile):
    """
    Iterate over hypotheses in a decoder hypothesis file.

    @param infile: File to iterate over.
    @type infile: file or str
    @return: Generator over (hyp, uttid, score) tuples
    @rtype: generator(str, str, int)
    """
    if isinstance(infile, file):
        infh = infile
    else:
        infh = file(infile)
    while True:
        hyp = infh.readline()
        if hyp == "":
            break
        yield parse_hyp(hyp)

countre = re.compile(r"(?:Words|Phones): (\S+) Correct: (\S+) Errors: (\S+) "
                     "Percent correct = (\S+)% Error = (\S+)% Accuracy = (\S+)%")
errtre = re.compile(r"Insertions: (\S+) Deletions: (\S+) Substitutions: (\S+)")
totalre = re.compile(r"TOTAL (?:Words|Phones)")
def alignfile(infile):
    """
    Iterate over alignments in the output of word_align.pl.

    This returns a generator over the alignment pairs in the alignment
    file.  Each element returned by the generator is a tuple
    consisting of:

     - The reference string, with *** inserted for insertions
     - The hypothesis string, with *** inserted for deletions
     - A dictionary containing statistics:
       - words: Number of reference words
       - correct: Number of correct words
       - errors: Number of errors (ins + del + subst)
       - %correct: Percent of words correct
       - %error: Word error rate
       - %accuracy: Accuracy (1 - WER)
       - insertions: Number of insertions
       - deletions: Number of deletions
       - substitutions: Number of substitutions

    @param infile: File to iterate over.
    @type infile: file or str
    @return: Generator over (ref, hyp, info) tuples
    @rtype: generator(str, str, dict)
    """
    if isinstance(infile, file):
        infh = infile
    else:
        infh = file(infile)
    while True:
        ref = infh.readline()
        if ref == "":
            break
        elif ref.find('word_align.pl') != -1:
            # Sometimes the command-line is on the first line of the file
            ref = infh.readline()
            if ref == "":
                break
        elif totalre.match(ref):
            break
        ref = ref.strip()
        hyp = infh.readline().strip()
        counts = infh.readline().strip()
        errtypes = infh.readline().strip()
        info = {}
        m = countre.match(counts)
        info['words'], info['correct'], info['errors'], \
                       info['%correct'], info['%error'], info['%accuracy'] \
                       = [float(x) for x in m.groups()]
        m = errtre.match(errtypes)
        info['insertions'], info['deletions'], info['substitutions'] \
                            = [float(x) for x in m.groups()]
        yield ref, hyp, info

alignre = re.compile(r"TOTAL Percent correct.*Error(?: =|:) (\S+)%")
def get_error_rates(alignfiles, xtransform=os.path.basename):
    """
    Retrieve error rates from a sequence or set of alignment files.
    Returns a list of X-axis values or labels and a list of error
    rates as percentages.
    """
    x = []
    y = []
    for f in alignfiles:
        for spam in file(f):
            m = alignre.match(spam)
            if m:
                x.append(xtransform(f))
                y.append(float(m.group(1)))
                break
    return x, y

