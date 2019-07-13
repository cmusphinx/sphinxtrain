#!/bin/bash
################################################################################################
#	Sphinx Acoustic Model Trainer script
# This script is to assist in training an acoustic model for
# pocketsphinx and sphinx4. Continuous or batch models may be made.
# Different training methods may be used.
# Use trainer.sh --help for more information.
#
# This script is not associated with, or created by the creators of Sphinx CMU.
# This author of this script/program/file is Tyler Sengia.
#
# Any damage, modifications, errors, side effects, etc that this script/program/file causes
# is at the liability of the user and not the author.
# By downloading/installing/running this script you agree to these terms.
#
###############################################################################################
#
# Based on the instructions from: https://cmusphinx.github.io/wiki/tutorialadapt/
# And instructions from: https://cmusphinx.github.io/wiki/tutorialtuning/
#

lineCount=21 # Number of lines to be read for the readings

#Variables and their default values
PROMPT_FOR_READINGS="yes"
DO_READINGS="yes"
DO_TESTS="yes"
DO_TEST_INITIAL="yes"
transcriptionFile="arctic20.transcription"
fileidsFile="arctic20.fileids"
CREATE_SENDUMP="no"
DO_MAP="yes"
DO_MLLR="yes"
SAMPLE_RATE=16000
LANGUAGE_MODEL="en-us.lm.bin"
DICTIONARY_FILE="cmudict-en-us.dict"
POCKET_SPHINX="no"

confirm() {
    # call with a prompt string or use a default
    echo " "
    read -r -p "${1:-Would you like to keep this recording? (No will start the recording over again.) [y/N]}" response
    case "$response" in
        [yY][eE][sS]|[yY])
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

doReadings() {
echo "Sphinx 4 Acoustic Library Auto Trainer"
echo "--------------------------------------"
echo "INSTRUCTIONS"
echo "A series of text will be displayed. Please recite the sentences to the best of your ability."
echo "When you are finished reciting the sentence, press any key."
echo "Continue reading the sentences until you have gone through all of them."
echo "Once all the sentences have been read, please wait a few moments for the trainer to run."
echo ""

#Take the transcription file and strip all of the <s> tags away and put it in a regular txt file to be read line by line
sed -r "s/<s>/ /g; s/<\/s>/ /g; s/\(.+\)/ /g" $transcriptionFile > transcription.txt


#Use the word count program (wc) to count the number of lines to be read
lineCount=`wc -l < transcription.txt`
echo "There are $lineCount sentences to be read."

for ((i=1; i < lineCount+1; i++))
do
readSentence
clear
cd recordings
mv ../output.wav `sed -n "$i{p;q}" ../$fileidsFile`.wav
cd ../
done
}

askForReadings() {
    # call with a prompt string or use a default
    read -r -p "${1:-Would you like to use the current audio files? (No will start the process of making new recordings.) [y/N]} " response
    case "$response" in
        [yY][eE][sS]|[yY])
            echo "no"
            ;;
        *)
            echo "yes"
            ;;
    esac
}

readSentence() {
	echo "Sentence $i"
	echo "Press ENTER when ready...."
	echo " "
	read
	sed -n "$i{p;q}" transcription.txt
	sleep 0.2

	{ arecord -c 1 -V mono -r $SAMPLE_RATE -f S16_LE output.wav & } 2>/dev/null

	childId=$!
	echo " "
	read -n 1 -s -r -p "[Press any key to finish recording]"
	echo " "
	{ kill -9 $childId && wait; } 2>/dev/null
	sleep 0.5
	if  confirm ; then
		return 1
	else
		clear
		readSentence
	fi
}

doContObservationCounts() {
echo "Accumulating observation counts..."
if [ ! -f $OUTPUT_MODEL/feature_transform ]; then
./bw -hmmdir $OUTPUT_MODEL \
 -moddeffn $OUTPUT_MODEL/mdef.txt \
 -ts2cbfn .cont. \
 -feat 1s_c_d_dd \
 -cmn current \
 -agc none \
 -dictfn $DICTIONARY_FILE \
 -ctlfn $fileidsFile \
 -lsnfn $transcriptionFile \
 -accumdir .
else
echo "Found feature-transform file! Using -lda option"
./bw -hmmdir $OUTPUT_MODEL \
 -moddeffn $OUTPUT_MODEL/mdef.txt \
 -ts2cbfn .cont. \
 -feat 1s_c_d_dd \
 -cmn current \
 -agc none \
 -dictfn $DICTIONARY_FILE \
 -ctlfn $fileidsFile \
 -lsnfn $transcriptionFile \
 -accumdir . \
 -lda $OUTPUT_MODEL/feature_transform
fi

echo " "
echo "Done accumulting observation counts."
}

doPtmObservationCounts() {
echo "Accumulating observation counts..."
if [ ! -f $OUTPUT_MODEL/feature_transform ]; then
./bw -hmmdir $OUTPUT_MODEL \
 -moddeffn $OUTPUT_MODEL/mdef.txt \
 -ts2cbfn .ptm. \
 -feat 1s_c_d_dd \
 -svspec 0-12/13-25/26-38 \
 -cmn current \
 -agc none \
 -dictfn $DICTIONARY_FILE \
 -ctlfn $fileidsFile \
 -lsnfn $transcriptionFile \
 -accumdir .
else
echo "Found feature-transform file! Using -lda option"
./bw -hmmdir $OUTPUT_MODEL \
 -moddeffn $OUTPUT_MODEL/mdef.txt \
 -ts2cbfn .ptm. \
 -feat 1s_c_d_dd \
 -svspec 0-12/13-25/26-38 \
 -cmn current \
 -agc none \
 -dictfn $DICTIONARY_FILE \
 -ctlfn $fileidsFile \
 -lsnfn $transcriptionFile \
 -accumdir . \
 -lda $OUTPUT_MODEL/feature_transform
fi

echo " "
echo "Done accumulting observation counts."
}

doSemiObservationCounts() {
echo "Accumulating observation counts..."
if [ ! -f $OUTPUT_MODEL/feature_transform ]; then
./bw -hmmdir $OUTPUT_MODEL \
 -moddeffn $OUTPUT_MODEL/mdef.txt \
 -ts2cbfn .semi. \
 -feat 1s_c_d_dd \
 -svspec 0-12/13-25/26-38 \
 -cmn current \
 -agc none \
 -dictfn $DICTIONARY_FILE \
 -ctlfn $fileidsFile \
 -lsnfn $transcriptionFile \
 -accumdir .
else
echo "Found feature-transform file! Using -lda option"
./bw -hmmdir $OUTPUT_MODEL \
 -moddeffn $OUTPUT_MODEL/mdef.txt \
 -ts2cbfn .semi. \
 -feat 1s_c_d_dd \
 -svspec 0-12/13-25/26-38 \
 -cmn current \
 -agc none \
 -dictfn $DICTIONARY_FILE \
 -ctlfn $fileidsFile \
 -lsnfn $transcriptionFile \
 -accumdir . \
 -lda $OUTPUT_MODEL/feature_transform
fi

echo " "
echo "Done accumulting observation counts."
}

makesendump() {
echo "Creating sendump file..."
./mk_s2sendump \
    -pocketsphinx $POCKET_SPHINX \
    -moddeffn $OUTPUT_MODEL/mdef.txt \
    -mixwfn $OUTPUT_MODEL/mixture_weights \
    -sendumpfn $OUTPUT_MODEL/sendump
echo " "
echo "Done creating sendump file."
}

doContMapUpdate() {
echo "Updating acoustic model files with MAP..."
./map_adapt \
    -moddeffn $OUTPUT_MODEL/mdef.txt \
    -ts2cbfn .cont. \
    -meanfn $OUTPUT_MODEL/means \
    -varfn $OUTPUT_MODEL/variances \
    -mixwfn $OUTPUT_MODEL/mixture_weights \
    -tmatfn $OUTPUT_MODEL/transition_matrices \
    -accumdir . \
    -mapmeanfn $OUTPUT_MODEL/means \
    -mapvarfn $OUTPUT_MODEL/variances \
    -mapmixwfn $OUTPUT_MODEL/mixture_weights \
    -maptmatfn $OUTPUT_MODEL/transition_matrices
echo " "
echo "Done updating with MAP."
}

doPtmMapUpdate() {
echo "Updating acoustic model files with MAP..."
./map_adapt \
    -moddeffn $OUTPUT_MODEL/mdef.txt \
    -ts2cbfn .ptm. \
    -meanfn $OUTPUT_MODEL/means \
    -varfn $OUTPUT_MODEL/variances \
    -mixwfn $OUTPUT_MODEL/mixture_weights \
    -tmatfn $OUTPUT_MODEL/transition_matrices \
    -accumdir . \
    -mapmeanfn $OUTPUT_MODEL/means \
    -mapvarfn $OUTPUT_MODEL/variances \
    -mapmixwfn $OUTPUT_MODEL/mixture_weights \
    -maptmatfn $OUTPUT_MODEL/transition_matrices
echo " "
echo "Done updating with MAP."
}

doSemiMapUpdate() {
echo "Updating acoustic model files with MAP..."
./map_adapt \
    -moddeffn $OUTPUT_MODEL/mdef.txt \
    -ts2cbfn .semi. \
    -meanfn $OUTPUT_MODEL/means \
    -varfn $OUTPUT_MODEL/variances \
    -mixwfn $OUTPUT_MODEL/mixture_weights \
    -tmatfn $OUTPUT_MODEL/transition_matrices \
    -accumdir . \
    -mapmeanfn $OUTPUT_MODEL/means \
    -mapvarfn $OUTPUT_MODEL/variances \
    -mapmixwfn $OUTPUT_MODEL/mixture_weights \
    -maptmatfn $OUTPUT_MODEL/transition_matrices
echo " "
echo "Done updating with MAP."
}

domllrupdate() {
echo "Updating acoustic model with MLLR..."
./mllr_solve \
    -meanfn $OUTPUT_MODEL/means \
    -varfn $OUTPUT_MODEL/variances \
    -outmllrfn mllr_matrix -accumdir .
echo ""
echo "Done updating with MLLR."
}

convertmdef() {
echo "Converting mdef into text format..."
pocketsphinx_mdef_convert -text $OUTPUT_MODEL/mdef $OUTPUT_MODEL/mdef.txt
echo " "
echo "Done converting mdef."
}

createAcousticFeatures() {
echo " "
echo "Creating acoustic feature files..."
sphinx_fe -argfile $OUTPUT_MODEL/feat.params -samprate $SAMPLE_RATE -c $fileidsFile -di ./recordings -do . -ei wav -eo mfc -mswav yes
echo "Done creating acoustic feature files."
echo " "
}

testInitialModel() {
echo "Testing acoustic model before adaptations..."
pocketsphinx_batch \
 -adcin yes \
 -cepdir ./recordings \
 -cepext .wav \
 -ctl $fileidsFile \
 -lm $LANGUAGE_MODEL \
 -dict $DICTIONARY_FILE \
 -hmm $OUTPUT_MODEL \
 -hyp initial_test.hyp
echo "Finished testing acoustic model before adaptations."
}

testFinalModel() {
echo "Testing acoustic model after adaptations..."
if [ $DO_MLLR = "no" ]; then
pocketsphinx_batch \
 -adcin yes \
 -cepdir ./recordings \
 -cepext .wav \
 -ctl $fileidsFile \
 -lm $LANGUAGE_MODEL \
 -dict $DICTIONARY_FILE \
 -hmm $OUTPUT_MODEL \
 -hyp final_test.hyp
else
pocketsphinx_batch \
 -adcin yes \
 -cepdir ./recordings \
 -cepext .wav \
 -ctl $fileidsFile \
 -lm $LANGUAGE_MODEL \
 -dict $DICTIONARY_FILE \
 -hmm $OUTPUT_MODEL \
 -hyp final_test.hyp \
 -mllr $OUTPUT_MODEL/mllr_matrix
fi
echo "Finished testing acoustic model after adaptations."
}

compareTests() {
echo "================================================================"
echo "BEFORE Adaptation:"
perl -w word_align.pl $transcriptionFile initial_test.hyp
echo "================================================================"
echo "AFTER Adaptation:"
perl -w word_align.pl $transcriptionFile final_test.hyp
echo "================================================================"
echo ""
}

displayHelp() {
cat << EOF
Sphinx Auto Trainer Script

Author: tylersengia@gmail.com

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

Issues, questions or suggestions: https://github.com/ExpandingDev/SphinxTrainingHelper
EOF
exit 2
}

#Parsing arguments
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -r|--readings)
    PROMPT_FOR_READINGS="no"
    DO_READINGS="$2"
    shift # past argument
    ;;
    --map)
    DO_MAP="$2"
    shift
    ;;
    --mllr)
    DO_MLLR="$2"
    shift
    ;;
    -s|--sample-rate)
    SAMPLE_RATE="$2"
    shift
    ;;
    -h|--help)
    displayHelp
    ;;
    -i|--test_initial)
    DO_TEST_INITIAL="$2"
    shift
    ;;
    -d|--dict)
    DICTIONARY_FILE="$2"
    shift
    ;;
    -p|--pocketsphinx)
    POCKET_SPHINX="$2"
    shift
    ;;
    --transcript)
    transcriptionFile="$2"
    shift
    ;;
    -f|--fileids)
    fileidsFile="$2"
    shift
    ;;
    --type)
    inputType="$2"
    shift
    ;;
    *)
    INPUT_MODEL="$1"
    OUTPUT_MODEL="$2"
    break
    ;;
esac
shift # past argument or value
done

#Sanitize the paths. Remove the ending / if there is one
OUTPUT_MODEL=${OUTPUT_MODEL%/}
INPUT_MODEL=${INPUT_MODEL%/}

#Check to see if we have all of the required programs/files and error out if we don't
if [ ! -f bw ]; then
    echo "ERROR: You are missing the 'bw' program in this directory. Please copy it into this directory from /usr/local/libexec/sphinxtrain (or wherever it is installed)."
    exit 1
fi

if [ ! -f map_adapt ]; then
    echo "ERROR: You are missing the 'map_adapt' program in this directory. Please copy it into this directory from /usr/local/libexec/sphinxtrain (or wherever it is installed)."
    exit 1
fi

if [ ! -f mllr_solve ]; then
    echo "ERROR: You are missing the 'mllr_solve' program in this directory. Please copy it into this directory from /usr/local/libexec/sphinxtrain (or wherever it is installed)."
    exit 1
fi

if [ ! -f mllr_transform ]; then
    echo "ERROR: You are missing the 'mllr_transform' program in this directory. Please copy it into this directory from /usr/local/libexec/sphinxtrain (or wherever it is installed)."
    exit 1
fi

if [ ! -f mk_s2sendump ]; then
    echo "ERROR: You are missing the 'mk_s2sendump' program in this directory. Please copy it into this directory from /usr/local/libexec/sphinxtrain (or wherever it is installed)."
    exit 1
fi

if [ ! -f word_align.pl ]; then
    echo "ERROR: You are missing the 'word_align.pl' perl script in this directory. Please copy it into this directory from sphinxtrain/scripts/decode (Extracted from the tar file. This script isn't installed, it comes straight from sphinxtrain's source files)."
    exit 1
fi

if [ ! -f $INPUT_MODEL/mixture_weights ]; then
    echo "ERROR: You are missing the 'mixture_weights' file in your input acoustic model. You may have to download the full version of the acoustic model that has the mixture_weights file present."
    echo "ERROR: Missing mixture_weights, cannot continue training, stopping..."
    exit 1
fi

#Check to see if we have all the necessary config/base files.
if [ ! -f $transcriptionFile ]; then
    echo "ERROR: The transcription file ($transcriptionFile) is missing!"
    exit 1
fi

if [ ! -f $fileidsFile ]; then
    echo "ERROR: The File IDs file ($fileidsFile) is missing!"
    exit 1
fi

if [ ! -f $LANGUAGE_MODEL ]; then
    echo "ERROR: The language model file ($LANGUAGE_MODEL) is missing!"
    exit 1
fi

if [ ! -f $DICTIONARY_FILE ]; then
    echo "ERROR: The dictionary ($DICTIONARY_FILE) is missing!"
    exit 1
fi

if [ ! -d $INPUT_MODEL ]; then
    echo "ERROR: Could not find the specified input/base acoustic model you specified: $INPUT_MODEL"
    exit 1
fi

# Check to see if the user entered a correct acoustic model type
case "$inputType" in
    [pP][tT][mM]|[pP])
        echo "Training a PTM acoustic model."
        modelType="PTM"
        ;;
    cont|[cC])
        echo "Training a continuous acoustic model."
        modelType="CONT"
        ;;
    semi|[sS])
        echo "Training a semi-continuous acoustic model."
        modelType="SEMI"
        ;;
    *)
        echo "Invalid model type supplied!!! Please include the --type argument or see --help for more details!"
        exit 1
        ;;
esac

# Test to make sure these aren't the same
if [ $OUTPUT_MODEL/feat.params -ef $INPUT_MODEL/feat.params ]; then
    echo "ERROR: Input and Output model paths are the same! This is not allowed!"
    exit 1
fi

cp -r $INPUT_MODEL $OUTPUT_MODEL

clear

#Ask to do the readings if the user didn't specify
if [ $PROMPT_FOR_READINGS = "yes" ]; then
    DO_READINGS=$(askForReadings)
fi

#Do the readings if the user wants to
if [ $DO_READINGS = "yes" ]; then
    #Clean up out directory before we begin
    rm -rf recordings
    rm transcription.txt
    mkdir recordings
    doReadings
fi

if [ $DO_TEST_INITIAL = "yes" ]; then
	testInitialModel
fi

createAcousticFeatures

#Convert the mdef file to txt filetype if it does not exist
if [ ! -f $OUTPUT_MODEL/mdef.txt ]; then
    convertmdef
fi

#Do observation counts, according to what type of acoustic model is being used
case "$modelType" in
    PTM)
        doPtmObservationCounts
        ;;
    CONT)
        doContObservationCounts
        ;;
    SEMI)
        doSemiObservationCounts
        ;;
    *)
        echo "Invalid model type supplied. This error should never ever happen! Impossible!"
        exit 1
        ;;
esac

# Copy fillerdict to noisedict if noisedict is missing
if [ ! -f $OUTPUT_MODEL/noisedict ]; then
    if [ -f fillerdict ]; then
    echo "Missing the 'noisedict' file, copying the 'fillerdict' file to use as noisedict"
    cp fillerdict $OUTPUT_MODEL/noisedict
    else
    echo "WARNING: Missing the 'noisedict' file as well as the 'fillerdict' file to replace the noisedict file. Press enter to continue."
    read
    fi
fi

#MLLR
if [ $DO_MLLR = "yes" ]; then
    echo "IMPORTANT: The MLLR Adaptation is supported in pocketsphinx but not sphinx4 (Java). It basically creates another config file to adjust all features. If using sphinx4, you need to use MAP adaptation."
    if [ $modelType = "SEMI" ]; then
        echo "WARNING: The MLLR Adaptation is not very effective for semi-continuous models because they rely on mixture weights. Press enter to continue."
    	read
    fi
    domllrupdate
    cp mllr_matrix $OUTPUT_MODEL/mllr_matrix
fi

#MAP
if [ $DO_MAP = "yes" ]; then
    if [ $modelType = "CONT" ]; then
        echo "WARNING: The MAP Adaptation requires lots of adaptation data to work effectively on continuous models. Press enter to continue."
        read
	doContMapUpdate
    fi
    if [ $modelType = "SEMI" ]; then
        doSemiMapUpdate
    fi
    if [ $modelType = "PTM" ]; then
        doPtmMapUpdate
    fi
fi
read -n 1 -s -p "Please read the above output thoroughly and then press any key to continue..."

makesendump

testFinalModel
echo " "
echo " "
compareTests

echo "DONE TRAINING."
