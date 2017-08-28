stseg_fldr="$1"
echo $stseg_fldr
files=$(find "$stseg_fldr/" -name "*.stseg")
for f in $files
do
	echo "CONVERTING: "$f
	cat $f | ../../src/libs/libcommon/.libs/stseg-read.o > $f.txt
	break
done
