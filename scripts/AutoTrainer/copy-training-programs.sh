#!/bin/bash
clear
echo "Please note, this is a very hackish way of copying over the needed programs. If this program fails to find the needed files, you should really do it yourself."
echo "Press any key to continue..."
read -n 1 -s
cp /usr/local/libexec/sphinxtrain/bw ./bw
cp /usr/local/libexec/sphinxtrain/map_adapt ./map_adapt
cp /usr/local/libexec/sphinxtrain/mllr_transform ./mllr_transform
cp /usr/local/libexec/sphinxtrain/mllr_solve ./mllr_solve
cp /usr/local/libexec/sphinxtrain/mk_s2sendump ./mk_s2sendump
echo "DONE"
echo " "
