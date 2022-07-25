# Training LibriSpeech with SphinxTrain

This document should (hopefully) explain how to train a basic
SphinxTrain model from the LibriSpeech dataset.  NOTE: It is a work in
progress and likely to change.

## Download and extract data

Download LibriSpeech from https://www.openslr.org/12 and
https://www.openslr.org/11 - you will need at least the
`train-clean-100` and `dev-clean` sections of the data, as well as the
lexicon and the sequitur g2p model, and one of the language models.
Unpack these all in the same directory.  It should look like this:

  $ ls
  3-gram.pruned.3e-7.arpa.gz
  LibriSpeech
  README.md
  dev-clean.tar.gz
  g2p-model-5
  librispeech-lexicon.txt
  train-clean-100.tar.gz

Now, in a *different* directory, initialize a SphinxTrain model:

  sphinxtrain -t librispeech setup

Link the the files extracted above to your training directory:

  ln -s ~/data/librispeech/LibriSpeech wav
  ln -s ~/data/librispeech/3-gram.pruned.3e-7.arpa.gz etc/
  ln -s ~/data/librispeech/librispeech-lexicon.txt etc/
  ln -s ~/data/librispeech/g2p-model-5 etc/

Edit the configuration to your liking.  You will at least need to
change CFG_FEATFILES_EXTENSION to "flac" and CFG_FEATFILES_TYPE to
"sox".

Now we will create the transcripts and the dictionaries using the
scripts in this directory.  First we generate the transcripts, which
will also create the list of OOV words:

  python3 scripts_pl/make_librispeech_transcripts.py \
  	  -l etc/librispeech-lexicon.txt \
	  --100 wav

Next we run Sequitur to generate pronunciations from the OOVs:

  g2p.py --model etc/g2p-model-5 etc/librispeech-100_train.oov \
  	 > etc/librispeech-100_train.oov.dic

Now we generate the full dictionary from the LibriSpeech lexicon and
the output of Sequitur:

  python3 scripts_pl/make_librispeech_dict.py \
  	  etc/librispeech-lexicon.txt

Great, we should be ready to run the training:

  sphinxtrain run

This will take a day or two on 4 CPUs.
