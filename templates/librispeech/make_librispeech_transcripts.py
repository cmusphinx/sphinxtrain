#!/usr/bin/env python3

"""
Convert LibriSpeech transcriptions to SphinxTrain format.
"""

import argparse
import os
import re


def find_dbname(cfgfile):
    dbname = re.compile(r"\$\s*CFG_DB_NAME\s*=\s*([\"'])([^\1]*)\1");
    with open(cfgfile, "rt") as fh:
        for spam in fh:
            m = dbname.search(spam)
            if m:
                return m.group(2)


def read_transcript(transtxt):
    """Read transcription and yield fileid and text."""
    with open(transtxt, "rt") as fh:
        for spam in fh:
            idx = spam.index(' ')
            fileid = spam[:idx].strip()
            text = spam[idx:].strip()
            yield fileid, text


def get_utts(corpus, dataset):
    """Get utterances from LibriSpeech dataset"""
    for root, dirs, files in os.walk(os.path.join(corpus, dataset)):
        for name in files:
            if name.endswith(".trans.txt"):
                for fileid, text in read_transcript(os.path.join(root, name)):
                    uttdir = os.path.relpath(root, corpus)
                    yield os.path.join(uttdir, fileid), text


def make_transcripts(corpus, sets, outbase, lexicon=None, oov=None):
    """Convert transcripts from LibriSpeech corpus"""
    with open(f"{outbase}.fileids", "wt") as outfds, \
         open(f"{outbase}.transcription", "wt") as outt:
        for dataset in sets:
            for uttpath, text in get_utts(corpus, dataset):
                print(uttpath, file=outfds)
                print(f"<s> {text} </s> ({uttpath})", file=outt)
                if oov is not None:
                    for w in text.split():
                        if w not in lexicon:
                            oov.add(w)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("corpus",
                        help="Path to LibriSpeech corpus")
    parser.add_argument("-l", "--lexicon",
                        help="Path to LibriSpeech lexicon, to find OOVs in training set")
    parser.add_argument("--dbname",
                        help="Name of training run (CFG_DB_NAME).  Will attempt to determine "
                        "from etc/sphinx_train.cfg if not given")
    parser.add_argument("-1", "--100", dest="t100",
                        action="store_true",
                        help="Include train-clean-100")
    parser.add_argument("-3", "--360", dest="t360",
                        action="store_true",
                        help="Include train-clean-360")
    parser.add_argument("-5", "--500", dest="t500",
                        action="store_true",
                        help="Include train-other-500")
    parser.add_argument("--other",
                        action="store_true",
                        help="Include dev-other and test-other")
    args = parser.parse_args()

    if args.dbname is None:
        args.dbname = find_dbname("etc/sphinx_train.cfg")
        if args.dbname is None:
            parser.error("Could not find dbname in etc/sphinx_train.cfg, please "
                         "pass it with --dbname")
        print("Found $CFG_DB_NAME:", args.dbname)

    if args.lexicon is not None:
        wordre = re.compile(r"^\s*(\S+)")
        lexicon = set()
        oov = set()
        with open(args.lexicon, "rt") as infh:
            for spam in infh:
                m = wordre.match(spam)
                lexicon.add(m.group(1))

    dev_sets = ["dev-clean"]
    if args.other:
        dev_sets.append("dev-other")
    print("Using dev sets:", " ".join(dev_sets))
    test_sets = ["test-clean"]
    if args.other:
        test_sets.append("test-other")
    print("Using test sets:", " ".join(test_sets))
    train_sets = []
    if args.t100:
        train_sets.append("train-clean-100")
    if args.t360:
        train_sets.append("train-clean-360")
    if args.t500:
        train_sets.append("train-other-500")
    print("Using train sets:", " ".join(train_sets))

    make_transcripts(args.corpus, dev_sets, f"etc/{args.dbname}_dev")
    make_transcripts(args.corpus, test_sets, f"etc/{args.dbname}_test")
    make_transcripts(args.corpus, train_sets, f"etc/{args.dbname}_train",
                     lexicon, oov)
    if args.lexicon is not None:
        with open(f"etc/{args.dbname}_train.oov", "wt") as fh:
            for w in sorted(oov):
                print(w, file=fh)
