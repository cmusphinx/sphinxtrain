# Copyright (c) 2009 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Filter dictionary to vocabulary from transcripts.

Creates a reduced dictionary containing only words that appear in the
training/test transcripts, preserving all pronunciation variants.
"""

import sys
from cmusphinx import s3dict


def load_vocab(vocab_path):
    """Load vocabulary from file (one word per line)."""
    vocab = set()
    with open(vocab_path) as fh:
        for line in fh:
            word = line.strip()
            if word:
                vocab.add(word)
    return vocab


def filter_dict(indict, vocab, outfh):
    """Filter dictionary to vocabulary, write to outfh."""
    in_words = set(indict.words())
    kept = vocab & in_words
    unused = in_words - vocab

    for w in sorted(kept):
        for i, phones in enumerate(indict.alts(w), 1):
            if i == 1:
                outfh.write("%s %s\n" % (w, " ".join(phones)))
            else:
                outfh.write("%s(%d) %s\n" % (w, i, " ".join(phones)))

    return kept, unused


def main():
    if len(sys.argv) < 4:
        print("Usage: %s DICT VOCAB OUTDICT" % sys.argv[0], file=sys.stderr)
        sys.exit(1)

    dict_path, vocab_path, out_path = sys.argv[1:4]

    vocab = load_vocab(vocab_path)
    indict = s3dict.open(dict_path)
    in_words = set(indict.words())

    with open(out_path, "w") as outfh:
        kept, unused = filter_dict(indict, vocab, outfh)

    # Stats
    full_prons = sum(indict.maxalt[w] for w in in_words)
    kept_prons = sum(indict.maxalt[w] for w in kept)

    print("Full dictionary:     %7d words" % len(in_words))
    print("Transcript vocab:    %7d words" % len(vocab))
    print("Kept in reduced:     %7d words" % len(kept))
    print("Removed (unused):    %7d words" % len(unused))
    print("Full pronunciations: %7d" % full_prons)
    print("Kept pronunciations: %7d" % kept_prons)

    # Write unused words for tracing
    unused_path = out_path.replace(".dic", ".unused")
    with open(unused_path, "w") as fh:
        for w in sorted(unused):
            fh.write("%s\n" % w)
    print("Unused words:        %s" % unused_path)

    # Warn about OOV
    oov = vocab - in_words
    if oov:
        print("OOV words:           %7d (missing from dict)" % len(oov),
              file=sys.stderr)


if __name__ == "__main__":
    main()
