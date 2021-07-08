#!/bin/bash
# Author: Tyler Sengia (ExpandingDev, tylersengia@gmail.com)
# https://github.com/ExpandingDev

#Filename of the generated transcription file. By default it is "generated.trans" Feel free to change this.
outputName="generated"

# String that prefixes the audio file ID number. See below for example. Reccommended to match your outputName and include an _ (underscore) to separate it from the number
# <s> blah blah blah this is a sentence </s> (audioFilePrefix0002)
audioFilePrefix="audio_"

echo "This is the generate_transcription_file.sh program. It is a file to help create the transcription file and fileids file to train CMU Sphinx acoustic models with."
echo "You will need to enter each transcription sentence by sentence when you are prompted."
echo "Close/exit the script (Ctrl + c) when you are done entering the transcription sentences."
echo "Output will be written to $outputName.trans and $outputName.fileids"

#Starting number for our audio files
index=0

while : ; do
read -e sentence

#This is setup to pad zeros to the number until it reaches 4 chars long (Ex: 0000 0001 0002 ... 0145) Change the %04d to %05d for 5 leading zeros
f=`printf "%04d" $index`
echo "<s> $sentence </s> ($audioFilePrefix$f)" >> $outputName.trans
echo "$audioFilePrefix$f" >> $outputName.fileids
index=$((index + 1))
done

echo "Exited"
