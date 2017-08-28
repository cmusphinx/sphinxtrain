stseg_fldr="$1"
read_fldr="$2"
echo $(pwd)
files=$(find "$stseg_fldr/" -name "*.stseg")
for f in $files
do
	echo "CONVERTING: "$f
	cat $f | $read_fldr/stseg-read > $f.txt
done
