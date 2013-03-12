#!/usr/bin/python
#
# Copyright (c) [2012-], Josef Robert Novak
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
#  modification, are permitted #provided that the following conditions
#  are met:
#
#  * Redistributions of source code must retain the above copyright 
#    notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above 
#    copyright notice, this list of #conditions and the following 
#    disclaimer in the documentation and/or other materials provided 
#    with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
# OF THE POSSIBILITY OF SUCH DAMAGE.
import re, operator, os
from collections import defaultdict
from calculateER import ErrorRater

def process_testset( testfile, wordlist_out, reference_out, verbose=False ):
    """
      Process the testfile, a normal dictionary, output a wordlist for testing,
      and a reference file for results evaluation.  Handles cases where a single
      word has multiple pronunciations.
    """

    if verbose: print "Preprocessing the testset dictionary file..."
    test_dict = defaultdict(list)
    for entry in open(testfile):
        try:
            items = entry.split()
            word = items[0]
            if word[-1] == ')':
                word = word[:-3]
            pron = " ".join(items[1:])
        except:
            continue
        test_dict[word].append(pron)

    wordlist_ofp  = open(wordlist_out,"w")
    reference_ofp = open(reference_out,"w")
    test_list = sorted(test_dict.iteritems(), key=operator.itemgetter(0))
    for entry in test_list:
        wordlist_ofp.write("%s\n" % entry[0])
        for p in entry[1]:
            reference_ofp.write("%s %s\n" % (entry[0], p))
    wordlist_ofp.close()
    reference_ofp.close()
    return

def evaluate_testset( 
    modelfile, wordlistfile, referencefile, hypothesisfile, pathtog2pdecoder, 
    verbose=False, ignore="", ignore_both=False, regex_ignore="", 
    beam=1500
    ):
    """
      Evaluate the Word Error Rate (WER) for the test set.
      Each word should only be evaluated once.  The word is counted as 
      'correct' if the pronunciation hypothesis exactly matches at least
      one of the pronunciations in the reference file.
      WER is then computed as:
         (1.0 - (WORDS_CORRECT / TOTAL_WORDS))
    """

    if verbose: print "Executing evaluation with command:"
    command = "%s -model %s -input %s -beam %d  -words yes -isfile yes -output_cost yes -output %s" \
        % (os.path.join(pathtog2pdecoder, "phonetisaurus-g2p"), modelfile, wordlistfile, beam, hypothesisfile)
#    print command
    os.system(command)
    references = {}
    for entry in open(referencefile,"r"):
#        parts = entry.strip().split("  ")
        parts = re.split(r' {2,}', entry.strip())
        word  = parts.pop(0)
        references[word] = parts
    for entry in open(hypothesisfile,"r"):
        #word, score, hypothesis = entry.strip().split("  ")
        word, score, hypothesis = re.split(r' {2,}', entry.strip())

    PERcalculator = ErrorRater( ignore=ignore, ignore_both=ignore_both, regex_ignore=regex_ignore )
    PERcalculator.compute_PER_phonetisaurus( hypothesisfile, referencefile, verbose=verbose )

    return


if __name__=="__main__":
    import sys, argparse

    decoder_path = sys.argv[1]
    g2p_model = sys.argv[2]
    test_file = sys.argv[3]
    prefix = sys.argv[4]

    wordlist = "%s.words" % ( prefix )
    hyp_file = "%s.hyp" % (prefix)
    ref_file = "%s.ref" % (prefix)

    process_testset( test_file, wordlist, ref_file )
    evaluate_testset(
        g2p_model, wordlist, ref_file, 
        hyp_file,  decoder_path)
