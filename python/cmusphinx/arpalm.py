# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read ARPA-format language models.

This module provides a class for reading, writing, and using ARPA
format statistical language model files.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from collections import defaultdict
import numpy
import gzip
import re
import os

LOG10TOLOG = numpy.log(10)
LOGTOLOG10 = 1./LOG10TOLOG

class SphinxLMCtl(object):
    """
    Language model control file used for Sphinx class language models
    and language model sets.
    """
    def __init__(self, infile=None):
        self.probdeffile = None
        self.lmfiles = {}
        self.classes = defaultdict(list)
        self.basedir = "."
        if infile != None:
            self.read(infile)

    def read(self, infile):
        """
        Read language model control file.
        """
        if not isinstance(infile, file):
            self.basedir = os.path.dirname(infile)
            infile = file(infile)
        # Format is:
        #
        # { probdef }
        #
        # arpafile lmname { classes }
        # ...
        def tokenize():
            for spam in infile:
                for x in spam.strip().split():
                    yield x
        def fail(msg):
            raise RuntimeError, msg
        t = tokenize()
        if t.next() != '{':
            fail("Expected {")
        self.probdeffile = os.path.join(self.basedir, t.next())
        if t.next() != '}':
            fail("Expected }")
        while True:
            try:
                arpafile = t.next()
                lmname = t.next()
                self.lmfiles[lmname] = os.path.join(self.basedir, arpafile)
                if t.next() != '{':
                    fail("Expected {")
                while True:
                    classname = t.next()
                    if classname == '}':
                        break
                    self.classes[lmname].append(classname)
            except StopIteration:
                break
                

class SphinxProbdef(object):
    """
    Probability definition file used for Sphinx class language models.
    """
    def __init__(self, infile=None):
        self.classes = {}
        if infile != None:
            self.read(infile)

    def read(self, infile):
        """
        Read probability definition from a file.
        """
        if not isinstance(infile, file):
            infile = file(infile)
        inclass = None
        for spam in infile:
            spam = spam.strip()
            if spam.startswith('#') or spam.startswith(';'):
                continue
            if spam == "":
                continue
            if inclass:
                parts = spam.split()
                if len(parts) == 2 \
                       and parts[0] == "END" and parts[1] == classname:
                    inclass = None
                else:
                    prob = 1.0
                    if len(parts) > 1:
                        prob = float(parts[1])
                    self.add_class_word(inclass, parts[0], prob)
            else:
                if spam.startswith('LMCLASS'):
                    foo, classname = spam.split()
                    self.add_class(classname)
                    inclass = classname

    def add_class(self, name):
        """
        Add a class to this probability definition.
        """
        self.classes[name] = {}

    def add_class_word(self, name, word, prob):
        """
        Add a word to a class in this probability definition.
        """
        self.classes[name][word] = prob
    
    def write(self, outfile):
        """
        Write out probability definition to a file.
        """
        if not isinstance(outfile, file):
            outfile = file(outfile)
        for c in self.classes:
            outfile.write("LMCLASS %s\n" % c)
            for word, prob in self.classes[c]:
                outfile.write("%s %g\n" % (word, prob))
            outfile.write("END %s\n" % c)
            outfile.write("\n")

    def normalize(self):
        """
        Normalize probabilities.
        """
        for c in self.classes:
            t = sum(self.classes[c].itervalues())
            if t != 0:
                for w in self.classes[c]:
                    self.classes[c][w] /= t

class ArpaLM(object):
    "Class for reading ARPA-format language models"
    class NGram(object):
        """
        Representation of a single N-Gram (only used for iteration)

        @ivar words: List of words
        @type words: tuple(str)
        @ivar log_prob: Log probability in base e
        @type log_prob: float
        @ivar log_bowt: Log backoff weight in base e
        @type log_bowt: float
        """
        __slots__ = ['words', 'log_prob', 'log_bowt']
        def __init__(self, words, log_prob=0, log_bowt=0):
            self.words = words
            self.log_prob = log_prob
            self.log_bowt = log_bowt

    def __init__(self, path=None, lw=1.0, wip=1.0):
        """
        Initialize an ArpaLM object.

        @param path: Path to an ARPA format file to (optionally) load
                     language model from.  This file can be
                     gzip-compressed if you like.
        @type path: string
        """
        if path != None:
            self.read(path)
        self.lw = lw
        self.wip = wip
        self.log_wip = numpy.log(wip)

    def read(self, path):
        """
        Load an ARPA format language model from a file in its entirety.

        @param path: Path to an ARPA format file to (optionally) load
                     language model from.  This file can be
                     gzip-compressed if you like.
        @type path: string
        """
        try:
            fh = gzip.open(path)
            fh.readline()
            fh.seek(0,0)
        except:
            fh = file(path)
        # Skip header
        while True:
            spam = fh.readline().rstrip()
            if spam == "\\data\\":
                break

        # Get N-gram counts
        self.ng_counts = {}
        r = re.compile(r"ngram (\d+)=(\d+)")
        while True:
            spam = fh.readline().rstrip()
            if spam == "":
                break
            m = r.match(spam)
            if m != None:
                n, c = map(int, m.groups())
                self.ng_counts[n] = c

        # Word and N-Gram to ID mapping
        self.ngmap = []
        # Create probability/backoff arrays
        self.n = max(self.ng_counts.keys())
        self.ngrams = []
        for n in range(1,self.n+1):
            vals = numpy.zeros((self.ng_counts[n],2),'d')
            self.ngrams.append(vals)
            self.ngmap.append({})

        # Read unigrams and create word id list
        spam = fh.readline().rstrip()
        if spam != "\\1-grams:":
            raise Exception, "1-grams marker not found"
        # ID to word mapping
        self.widmap = []
        wordid = 0
        while True:
            spam = fh.readline().rstrip()
            if spam == "":
                break
            p,w,b = spam.split()
            self.ngmap[0][w] = wordid
            self.widmap.append(w)
            self.ngrams[0][wordid,:] = (float(p) * LOG10TOLOG,
                                        float(b) * LOG10TOLOG)
            wordid = wordid + 1
        
        # Read N-grams
        r = re.compile(r"\\(\d+)-grams:")
        ngramid = 0
        # Successor list map
        self.succmap = {}
        while True:
            spam = fh.readline().rstrip()
            if spam == "":
                continue
            if spam == "\\end\\":
                break
            m = r.match(spam)
            if m != None:
                n = int(m.group(1))
                ngramid = 0
            else:
                spam = spam.split()
                p = float(spam[0]) * LOG10TOLOG
                if len(spam) == n + 2:
                    ng = tuple(spam[1:-1])
                    b = float(spam[-1]) * LOG10TOLOG
                elif len(spam) == n + 1:
                    ng = tuple(spam[1:])
                    b = 0.0
                else:
                    raise RuntimeError, "Found %d-gram in %d-gram section" % (len(spam)-1, n)
                # N-Gram info
                self.ngrams[n-1][ngramid,:] = p, b
                self.ngmap[n-1][ng] = ngramid

                # Successor list for N-1-Gram
                mgram = tuple(ng[:-1])
                if mgram not in self.succmap:
                    self.succmap[mgram] = []
                self.succmap[mgram].append(ng[-1])
                ngramid = ngramid + 1

    def get_size(self):
        """
        Get the order (i.e. N) of this N-Gram model.

        @return: Order of this model.
        @rtype: int
        """
        return len(self.ngmap)

    def save(self, path):
        """
        Save an ARPA format language model to a file.

        @param path: Path to save the file to.  If this ends in '.gz',
                     the file contents will be gzip-compressed.
        @type path: string
        """
        if path.endswith('.gz'):
            fh = gzip.open(path, 'w')
        else:
            fh = open(path, 'w')
        fh.write("# Written by arpalm.py\n")
        fh.write("\\data\\\n")
        for n in range(1, self.n+1):
            fh.write("ngram %d=%d\n" % (n, self.ng_counts[n]))
        for n in range(1, self.n+1):
            fh.write("\n\\%d-grams:\n" % n)
            ngrams = self.ngmap[n-1].keys()
            ngrams.sort()
            if '<UNK>' in self.ngmap[n-1]:
                ngid = self.ngmap[n-1]['<UNK>']
                score, bowt = self.ngrams[n-1][ngid]
                score *= LOGTOLOG10
                bowt *= LOGTOLOG10
                if n == self.n:
                    fh.write("%.4f <UNK>\n" % (score))
                else:
                    fh.write("%.4f <UNK>\t%.4f\n" % (score,bowt))
            for g in ngrams:
                if g == '<UNK>':
                    continue
                ngid = self.ngmap[n-1][g]
                score, bowt = self.ngrams[n-1][ngid]
                score *= LOGTOLOG10
                bowt *= LOGTOLOG10
                if n > 1:
                    g = " ".join(g) 
                if n == self.n:
                    fh.write("%.4f %s\n" % (score, g))
                else:
                    fh.write("%.4f %s\t%.4f\n" % (score, g, bowt))
        fh.write("\n\\end\\\n")
        fh.close()

    def ngram(self, word, *hist):
        """
        Get the N-gram record for word with given history.

        As with prob() and score(), the history is given in reverse order.
        """
        syms = tuple(reversed((word,) + hist))
        if len(syms) == 1:
            ngid = self.ngmap[0][syms[0]]
        else:
            ngid = self.ngmap[len(syms)-1][syms]
        return self.NGram(syms, *self.ngrams[len(syms)-1][ngid])

    def mgrams(self, m):
        """
        Return an iterator over N-Grams of order M+1.

        @param m: Length of history (i.e. order-1) of desired N-Grams.
        @type m: int
        @return: Iterator over N-Grams
        @rtype: generator(NGram)
        """
        for ng, ngid in self.ngmap[m].iteritems():
            if isinstance(ng, str):
                ng = (ng,)
            yield self.NGram(ng, *self.ngrams[m][ngid,:])

    def successor_words(self, words):
        """
        Return all successor words for a word-tuple

        @param words: A sequence of words.
        @type words: sequence of words
        @return: A generator over successor words
        @rtype: generator(str)
        """
        if isinstance(words, str):
            words = (words,)
        else:
            words = tuple(words)
        if words in self.succmap:
            for w in self.succmap[words]:
                yield w
        
    def successors(self, ng):
        """
        Return all successors for an M-Gram

        @param ng: An Ngram as returned by mgrams()
        @type ng: NGram
        @return: An iterator over all (M+1)-Gram successors to ng.
        @rtype: generator(NGram)
        """
        if ng.words in self.succmap:
            for w in self.succmap[ng.words]:
                succ = ng.words + (w,)
                ngid = self.ngmap[len(succ)-1][succ]
                yield self.NGram(ng.words + (w,),
                                 *self.ngrams[len(succ)-1][ngid])
                
    def score(self, *syms):
        p = self.prob(*syms)
        return p * self.lw + self.log_wip

    def prob(self, *syms):
        """
        Return the language model log-probability for an N-Gram
        (passed in reverse order, possibly with extra history)

        @return: The log probability for the N-Gram consisting of the
                 words given, in base e (natural log).
        @rtype: float
        """
        syms = syms[0:min(len(syms),self.n)]
        # It makes the most sense to do this recursively
        n = len(syms)
        if n == 1:
            if syms[0] in self.ngmap[0]:
                # 1-Gram exists, just return its probability
                return self.ngrams[0][self.ngmap[0][syms[0]]][0]
            elif '<UNK>' in self.ngmap[0]:
                # Use <UNK>
                return self.ngrams[0][self.ngmap[0]['<UNK>']][0]
            else:
                raise IndexError, "Unknown unigram %s" % syms[0]
        else:
            # Forward N-gram (since syms is reversed)
            fsyms = tuple(reversed(syms))
            if fsyms in self.ngmap[n-1]:
                # N-Gram exists, just return its probability
                return self.ngrams[n-1][self.ngmap[n-1][fsyms]][0]
            else:
                # Backoff: alpha(history) * probability (N-1-Gram)
                fhist = fsyms[:-1]
                # New N-1 gram symbols (reversed order)
                syms = syms[:-1]
                # Treat unigram histories a bit specially
                if len(fhist) == 1:
                    fhist = fhist[0]
                    # Try to back off to <UNK> if word doesn't exist -
                    # if this is a closed vocab model this will just
                    # return the unigram prob for syms[0]
                    if not fhist in self.ngmap[0]:
                        fhist = '<UNK>'
                if fhist in self.ngmap[n-2]:
                    # Try to use the history if it exists
                    bowt = self.ngrams[n-2][self.ngmap[n-2][fhist]][1]
                    return bowt + self.prob(*syms)
                else:
                    # Otherwise back off some more
                    return self.prob(*syms)

    def adapt_rescale(self, unigram, vocab=None):
        """Update unigram probabilities with unigram (assumed to be in
        linear domain), then rescale N-grams ending with the same word
        by the corresponding factors.  If unigram is not the same size
        as the original vocabulary, you must pass vocab, which is a
        list of the words in unigram, in the same order as their
        probabilities are listed in unigram."""
        if vocab:
            # Construct a temporary list mapping for the unigrams
            vmap = map(lambda w: self.ngmap[0][w], vocab)
            # Get the original unigrams
            og = numpy.exp(self.ngrams[0][:,0].take(vmap))
            # Compute the individual scaling factors
            ascale = unigram * og.sum() / og
            # Put back the normalized version of unigram
            self.ngrams[0][:,0].put(numpy.log(unigram * og.sum()), vmap)
            # Now reconstruct vocab as a dictionary mapping words to
            # scaling factors
            vv = {}
            for i, w in enumerate(vocab):
                vv[w] = i
            vocab = vv
        else:
            ascale = unigram / numpy.exp(self.ngrams[0][:,0])
            self.ngrams[0][:,0] = numpy.log(unigram)

        for n in range(1, self.n):
            # Total discounted probabilities for each history
            tprob = numpy.zeros(self.ngrams[n-1].shape[0], 'd')
            # Rescaled total probabilities
            newtprob = numpy.zeros(self.ngrams[n-1].shape[0], 'd')
            # For each N-gram, accumulate and rescale
            for ng,idx in self.ngmap[n].iteritems():
                h = ng[0:-1]
                if n == 1: # Quirk of unigrams
                    h = h[0]
                w = ng[-1]
                prob = numpy.exp(self.ngrams[n][idx,0])
                tprob[self.ngmap[n-1][h]] += prob
                if vocab == None or w in vocab:
                    prob = prob * ascale[vocab[w]]
                newtprob[self.ngmap[n-1][h]] += prob
                self.ngrams[n][idx,0] = numpy.log(prob)
            # Now renormalize everything
            norm = tprob / newtprob
            for ng,idx in self.ngmap[n].iteritems():
                h = ng[0:-1]
                if n == 1: # Quirk of unigrams
                    h = h[0]
                w = ng[-1]
                prob = numpy.exp(self.ngrams[n][idx,0])
                self.ngrams[n][idx,0] = numpy.log(prob * norm[self.ngmap[n-1][h]])
