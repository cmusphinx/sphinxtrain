#!/bin/bash
echo "This file is designed to delete un-needed files from the current directory to get a new iteration of training ready."
read -n 1 -s -p "Press any key to start..."
rm *.mfc
rm *.hyp
rm tmat_counts
rm mixw_counts
rm gauden_counts
rm mllr_matrix
echo "DONE"
