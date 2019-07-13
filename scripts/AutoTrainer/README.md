# Sphinx Training Helper
A Bash script designed to make training sphinx4 and pocketsphinx acoustic libraries faster and easier.

This script is not created by the authors of Sphinx CMU or related software and assets.

# Installation
Sphinx Training Helper uses the `arecord` command during Readings Mode. Please ensure that ALSA is installed on your machine and configured properly in order to use Readings.  

If you would like to use the random transcript generator, you will need the `hxnormalize hxselect lynx` commands available. Installing the `html-xml-utils` and `lynx` packages should do the trick. (`sudo apt-get install -y html-xml-utils lynx`).

The Sphinx CMU toolkit should be downloaded and installed on your machine. This includes: sphinxbase, pocketsphinx, and sphinx_train.  

In order to train/update the acoustic model, this script will need the following programs in the same directory: 
bw, map_adapt, mllr_solve, mllr_transform, mk_s2sendump, word_align.pl  

These programs/binaries can be found where you installed sphinx_train (on Linux this should be `/usr/local/libexec/sphinxtrain`, for more information, see the tutorial on CMU Sphinx's website. Simply copy the needed executables from that directory to the same directory as the Sphinx Training Helper.
However, there is also a simple bash script (`copy-training-programs.sh`) that is included that can be used to copy the needed programs into the directory.

Additionally, the `word_align.pl` script is needed to test the effectiveness of the acoustic model adaptation. You will need to copy it from your `sphinx_train/scripts/decode` directory. As of now the `copy-training-programs.sh` script does not copy in the `word_align.pl` script.

# Instructions
          Usage: trainer [OPTIONS] --type TYPE input_model output_model
                    input_model : The directory/filename of acoustic model to create the trained acoustic model off of.
                    output_model : The desired name of the trained acoustic model that will be created using this script.

          TYPE may be any of:
              p   PTM
              c   continuous
              s   semi-continuous


          OPTIONS may be any of:
                    -h	--help			Displays this help text and exits.
                    -r	--readings {yes|no}	Enable or disable sentence reading. Disabling sentence reading means that the audio files in the working directory (as referenced by the fileids) will be used to train.
                    -s	--sample_rate {int}	Specify the sample rate for the audio files. Expand the value (ie 16kHz should be 16000). Default is 16000.
                              --map {yes|no}		Enable or disable MAP weight updating. Supported in pocketsphinx and shpinx4. Default is yes.
                              --mllr {yes|no}		Enable or disable MLLR weight updating. Currently only supported in pocketsphinx Default is yes.
                              --transcript {file}	Specify the transcript file for readings. (default: arctic20.transcription)
                              --type    TYPE        Specify what TYPE of acoustic model is being trained. See above for valid identifiers.
                    -i	--test_initial {yes|no}	Specifiy whether or not to test the initial acoustic model before adaptation. This can help save time. Default is yes.
                    -f	--fileids {file}	Specify the fileids file for readings. (default: acrtic20.fileids)
                    -p	--pocketsphinx {yes|no} Specfiy whether or not you are training the model for pocket sphinx. Specifying yes will add optimizations. Default is yes. Set to "no" if using for Sphinx4 (Java).
                    -d  --dict                      Specify the path to the dictionary to use. Default is "cmudict-en-us.dict"
            
# Readings Mode
The so called "Readings Mode" in this script is a simple command line interface that allows the user to read the entire transcript file line by line while recording. If readings mode is enabled, the user will be displayed a line from the transcript file that should be read aloud. When the user is done reading the line, they can press any key to stop recording. The user is then prompted to either user the recording or redo the recording. Once all lines from the transcript are read, the script will begin adapting the acoustic model.  

The purpose of Readings Mode is to make recording quick, simple, and painless for the user. No need to open up Audacity or a recording program and splice audio recordings, just read and press the enter key.

Readings Mode is enabled by default as the `trainer.sh` script assumes that the user wants to record new audio files. To turn the Readings Mode off use the `-r` option.  
Example: `./trainer.sh -r no -p yes --type c -f arctic20.fileids --transcript arctic20.trans input-model/ output-model/`

# Transcription File and File IDs File
In this repository there is the `generate_transcription_file.sh` script that can be use to quickly create a transcription file and a fileids file. This script allows the user to enter sentences one at a time (pressing enter after each), which will append sentences to the generated transcript and fileids file. To change the name of the generated transcript and fileids files, you will need to change the value of the `outputName` variable. It's value is set within the first few lines of the script, so it should be very easy to find. Also, the fileids file contains references to audiofile names. The audio file names generated by default are `audio_0000`. To change the generated audio file name to `example_0000`, change the `audioFilePrefix` variable's value to `"example_"`. The script will always append a 4 character wide, zero padded ID number to the audio file name in the generated fileids file and in the transcript file entries.   

The CMU Sphinx website provides examples for writing transcript and file IDs files, but here are the formats anyways.

### Transcription File
A text file containing the words that will be/are spoken in an audio file.  
The words should be grouped into sentences as marked by an XML-like `<s> Your words go here </s>` tag.
Following the `<s>` tag should be a space and a set of parenthesis with the audio file name inside (without the extension).  
For example:  

    <s> hello world this is an example transcription file </s> (audiofile_0001)
    <s> this is the second sentence in the transcription file </s> (audiofile_0002)
    <s> we can even add a third sentence </s> (audiofile_0003)
    <s> just remember to increment the file id in the parenthesises </s> (audiofile_0004)
    
### File IDs File
The File IDs file is simply a text file where each line contains the file name of an audio file (do not include the extension).  
The file names should be listed in the same order as the transcription file.  
Remember to increment any numbers identifying the audio files.
Example File IDs file:  

    audiofile_0001
    audiofile_0002
    audiofile_0003
    audiofile_0004
    audiofile_0005
    
## `random_trainer.sh`
The `random_trainer.sh` script is a bit of a hack, but it is a very useful script. 
The script scrapes a website that generates random sentences and uses that sentence as part of the transcript that the user reads. This allows for the user to adapt the acoustic model without manually creating a fileids file or a transcript file. The script will automatically generate the filesids file and transcript file as the user goes through the Readings Mode. 
You will need an internet connection.  

The command also takes a third argument after the input and output model directories.  
Example: `./random_trainer.sh -p yes -r yes -i no --type c intput-model/ output-mode/ 21`  
The above example command will scrape a website for 21 random sentences and generate a transcript and fileids file using those sentences. Notice that the transcript and fileids arguments do not have to be supplied as the files are generated automatically and used immediately afterwards. By default the generated files will be `random.transcript` and `random.fileids`, but these can be changed by using the `--transcript` and `-f` options.

### `-i` Option
By default, before running the adaptation commands, the trainer script will test the initial acoustic model. This is to show the user in the end how much accuracy has (or hasn't) increased. This is useful to monitor that adaptation is working, however it is also a very long process, and is sometimes uncessesary (sometimes you know that your initial accuracy is terrible, so why test it again?). To save some time, pass the trainer script the `-i no` option.  
Example: `./trainer.sh -i no -p yes -r no --transcript arctic20.trans -f arctic20.fileids --type c input-model/ output-model/`
