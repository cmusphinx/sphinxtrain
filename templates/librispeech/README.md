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

Create a scratch directory for training and initialize it for
SphinxTrain:

    mkdir /path/to/scratch/librispeech # change this!
    cd /path/to/scratch/librispeech
    sphinxtrain -t librispeech setup

Link some files to it (change `~/data/librispeech` to wherever you
extracted the LibriSpeech files above):

    ln -s ~/data/librispeech/LibriSpeech wav # or from wherever
    ln -s ~/data/librispeech/3-gram.pruned.3e-7.arpa.gz etc/
    ln -s ~/data/librispeech/librispeech-lexicon.txt etc/

Edit a few things in `sphinx_train.cfg`:

    $CFG_WAVFILE_EXTENSION = 'flac';
    $CFG_WAVFILE_TYPE = 'sox';
    $CFG_HMM_TYPE  = '.ptm.';
    $CFG_INITIAL_NUM_DENSITIES = 64; # line 137, under "elsif... ptm"
    $CFG_FINAL_NUM_DENSITIES = 64;
    $CFG_N_TIED_STATES = 5000;
    $CFG_NPART = 4;
    $CFG_QUEUE_TYPE = "Queue::POSIX";
    $CFG_G2P_MODEL= 'yes';
    $DEC_CFG_LANGUAGEMODEL  = "$CFG_BASE_DIR/etc/3-gram.pruned.3e-7.arpa.gz";
    $DEC_CFG_LISTOFFILES    = "$CFG_BASE_DIR/etc/${CFG_DB_NAME}_dev.fileids";
    $DEC_CFG_TRANSCRIPTFILE = 
        "$CFG_BASE_DIR/etc/${CFG_DB_NAME}_dev.transcription";
    $DEC_CFG_NPART = 8;

Now we will create the transcripts and the dictionaries.
Pronunciation generation for the OOV words will be handled
automatically, provided you have compiled SphinxTrain with
`--enable-g2p-decoder`.

    python3 make_librispeech_transcripts.py \
	    -l etc/librispeech-lexicon.txt \
	    --100 wav
    python3 make_librispeech_dict.py etc/librispeech-lexicon.txt

Great, we should be ready to run the training:

    sphinxtrain run

This will take a couple days on 4 CPUs.
