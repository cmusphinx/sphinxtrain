#!/usr/bin/env python3

"""
Create the dictionary and phoneset files for training librispeech.
"""

from collections import defaultdict
from make_librispeech_transcripts import find_dbname
import argparse
import fileinput
import re

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("lexicon", help="LibriSpeech lexicon file")
    parser.add_argument("--dbname",
                        help="Name of training run (CFG_DB_NAME).  Will attempt to determine "
                        "from etc/sphinx_train.cfg if not given")
    args = parser.parse_args()

    if args.dbname is None:
        args.dbname = find_dbname("etc/sphinx_train.cfg")
        if args.dbname is None:
            parser.error("Could not find dbname in etc/sphinx_train.cfg, please "
                         "pass it with --dbname")
        print("Found $CFG_DB_NAME:", args.dbname)

    words = []
    nprons = defaultdict(int)
    phoneset = set()
    remove_stress = re.compile(r"\d$")

    def read_lexicon(path):
        with open(path, "rt") as fh:
            for line in fh:
                fields = line.strip().split()
                word = fields[0]
                phones = fields[1:]
                nprons[word] += 1
                if nprons[word] > 1:
                    word = f"{word}({nprons[word]})"
                base_phones = []
                for ph in phones:
                    base_phone = remove_stress.sub("", ph)
                    phoneset.add(base_phone)
                    base_phones.append(base_phone)
                words.append([word] + base_phones)

    read_lexicon(args.lexicon)
    try:
        read_lexicon(f"etc/{args.dbname}_train.oov.dic")
    except FileNotFoundError:
        pass

    words.sort()
    with open(f"etc/{args.dbname}.dic", "wt") as fh:
        for line in words:
            print(" ".join(line), file=fh)
    with open(f"etc/{args.dbname}.filler", "wt") as fh:
        # FIXME: Copy it from PocketSphinx in force alignment
        fh.write("""<s> SIL
</s> SIL
<sil> SIL
""")
    phoneset.add("SIL")
    with open(f"etc/{args.dbname}.phone", "wt") as fh:
        for ph in sorted(phoneset):
            print(ph, file=fh)
