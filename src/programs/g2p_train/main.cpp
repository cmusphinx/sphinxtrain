#include <iostream>
#include <vector>
#include <string>
#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>
#include "g2p_train.hpp"

using namespace std;

const char helpstr[] =
		"Usage: g2p_train [-seq1_del] [-seq2_del] [-seq1_max SEQ1_MAX] [-seq2_max SEQ2_MAX] \n\ 
	               [-seq_sep SEQ_SEP] [-s1s2_sep S1S2_SEP] [-eps EPS]   \n\
	               [-skip SKIP] [-seq1in_sep SEQ1IN_SEP] [-seq2in_sep SEQ2IN_SEP]  \n\
	               [-s1s2_delim S1S2_DELIM] [-iter ITER] [-order ORDER] [-smooth SMOOTH]  \n\
	               [-prune PRUNE] [-theta THETA] [-pattern PATTERN] [-noalign]  \n\
	               [-ratio RATIO] -ifile IFILE -prefix PREFIX \n\
	\n\
	  -seq1_del,              Allow deletions in sequence 1. Defaults to false.  \n\
	  -seq2_del,              Allow deletions in sequence 2. Defaults to false.  \n\
	  -seq1_max SEQ1_MAX,     Maximum subsequence length for sequence 1. Defaults to 2. \n\
	  -seq2_max SEQ2_MAX,     Maximum subsequence length for sequence 2. Defaults to 2. \n\
	  -seq_sep SEQ_SEP,       Separator token for sequences 1 and 2. Defaults to '|'.  \n\
	  -s1s2_sep S1S2_SEP,     Separator token for seq1 and seq2 alignments. Defaults to '}'. \n\
	  -eps EPS,               Epsilon symbol.  Defaults to '<eps>'. \n\
	  -skip SKIP,             Skip/null symbol.  Defaults to '_'.  \n\
	  -seq1in_sep SEQ1IN_SEP, Separator for seq1 in the input training file. Defaults to ''.  \n\
	  -seq2in_sep SEQ2IN_SEP, Separator for seq2 in the input training file. Defaults to ' '.  \n\
	  -s1s2_delim S1S2_DELIM, Separator for seq1/seq2 in the input training file. Defaults to '  '. \n\
	  -iter ITER,             Maximum number of iterations for EM. Defaults to 10. \n\
	  -order ORDER,           N-gram order. Defaults to 6.  \n\
	  -smooth SMOOTH,         Smoothing method. Available options are: \n\
	                          \"no\", \"presmoothed\", \"unsmoothed\", \"kneser_ney\", \"absolute\",  \n\
	                          \"katz\", \"witten_bell\". Defaults to \"kneser_ney\". \n\
	  -prune PRUNE,           Prunning method. Available options are:  \n\
	                          \"no\", \"count_prune\", \"relative_entropy\", \"seymore\". Defaults to \"no\".  \n\
	  -theta THETA,           Theta value for \"relative_entropy\" and \"seymore\" prunning. Defaults to 0 (ie no pruning).  \n\
	  -pattern PATTERN,       Count cuttoffs for the various n-gram orders for \"count_prune\" prunning. Defaults to \"\".  \n\
	  -noalign,               Do not align. Assume that the aligned corpus already exists. \n\
	                          Defaults to false. \n\
	  -ifile IFILE,           File containing sequences to be aligned.  \n\
	  -gen_testset		  	  Whether or not a testset will be left out from the input dictionary, for model evaluation. Defaults to true. \n\
   	  -ratio RATIO		  	  if a testset is generated, 1 word in every RATIO words will be left out from the input dictionary and inserted to the test set for model evaluation. Defaults to 10. \n\
	  -prefix PREFIX,         Prefix for saving the generated model and other files. Defaults to \"./model\"";

	  int main(int argc, char* argv[]) {
	      static arg_t defn[] = {
	      		{ "-help",		 ARG_BOOLEAN,	"no",	  		"Shows the usage of the tool"},
	  			{ "-seq1_del",	 ARG_BOOLEAN,	"no",			"Allow deletions in sequence 1. Defaults to false." },
	  			{ "-seq2_del",	 ARG_BOOLEAN,	"no", 			"Allow deletions in sequence 2. Defaults to false." },
	  			{ "-noalign",	 ARG_BOOLEAN,	"no", 			"Do not align. Assume that the aligned corpus already exists. Defaults to false." },
	  			{ "-seq1_max",	 ARG_INT32,		"2", 			"Maximum subsequence length for sequence 1. Defaults to 2." },
	  			{ "-seq2_max",	 ARG_INT32,		"2",			"Maximum subsequence length for sequence 2. Defaults to 2." },
	  			{ "-seq_sep",	 ARG_STRING,	"|",			"Separator token for sequences 1 and 2. Defaults to '|'." },
	  			{ "-s1s2_sep",	 ARG_STRING,	"}",			"Separator token for seq1 and seq2 alignments. Defaults to '}'" },
	  			{ "-eps",		 ARG_STRING,	"<eps>",		"Epsilon symbol.  Defaults to '<eps>'." },
	  			{ "-skip", 		 ARG_STRING,	"_",			"Skip/null symbol.  Defaults to '_'." },
	  			{ "-seq1in_sep", ARG_STRING,	"",				"Separator for seq1 in the input training file. Defaults to ''." },
	  			{ "-seq2in_sep", ARG_STRING,	" ",			"Separator for seq2 in the input training file. Defaults to ' '." },
	  			{ "-s1s2_delim", ARG_STRING,	"  ",			"Separator for seq1/seq2 in the input training file. Defaults to '  '." },
	  			{ "-iter", 		 ARG_INT32,		"10",			"Maximum number of iterations for EM. Defaults to 10." },
	  			{ "-order",		 ARG_INT32,		"6",			"N-gram order. Defaults to 6." },
	  			{ "-prune",		 ARG_STRING,	"no",			"Prunning method. Available options are: 'no', 'count_prune', 'relative_entropy', 'seymore'. Defaults to 'no'." },
	  			{ "-theta",		 ARG_FLOATING,	"0",			"Theta value for 'relative_entropy' and 'seymore' prunning. Defaults to 0 (ie no pruning)." },
	  			{ "-pattern",	 ARG_STRING,	"", 			"Count cuttoffs for the various n-gram orders for 'count_prune' prunning. Defaults to ''." },
	  			{ "-smooth",	 ARG_STRING,	"kneser_ney",	"Smoothing method. Available options are: 'presmoothed', 'unsmoothed', 'kneser_ney', 'absolute', 'katz', 'witten_bell'. Defaults to 'kneser_ney'." },
	  			{ "-ifile",		 REQARG_STRING,	"",		      	"The input dictionary file." },
	  			{ "-gen_testset",	 ARG_BOOLEAN,	"yes",		"Whether or not a testset (1 in every 10 words) will be left out from the input dictionary, for model evaluation. Defaults to true." },
	  			{ "-prefix",	 ARG_STRING,	"model",		"Prefix for saving the generated model and other files. Defaults to 'model'" },
	  			{ "-ratio",  	 ARG_INT32, "10",		  	    "If a testset is generated, 1 word in every RATIO words will be left out from the input dictionary and inserted to the test set for model evaluation. Defaults to 10. " },
	  			{ NULL, 		 0,				NULL, 			NULL }
	  	};

	    cmd_ln_parse(defn, argc, argv, TRUE);

	    if(cmd_ln_int32("-help")) {
	      	printf("%s\n\n",helpstr);
	    }

	    bool seq1_del = cmd_ln_boolean("-seq1_del");
	    bool seq2_del = cmd_ln_boolean("-seq2_del");
	    bool noalign = cmd_ln_boolean("-noalign");
	    bool gen_testset = cmd_ln_boolean("-gen_testset");
	  	int seq1_max = cmd_ln_int32("-seq1_max");
	  	int seq2_max = cmd_ln_int32("-seq1_max");
	  	string seq_sep = cmd_ln_str("-seq_sep");
	  	string s1s2_sep = cmd_ln_str("-s1s2_sep");
	  	string eps = cmd_ln_str("-eps");
	  	string skip = cmd_ln_str("-skip");
	  	string seq1in_sep = cmd_ln_str("-seq1in_sep");
	  	string seq2in_sep = cmd_ln_str("-seq2in_sep");
	  	string s1s2_delim = cmd_ln_str("-s1s2_delim");
	  	int iter = cmd_ln_int32("-iter");
	  	int ratio = cmd_ln_int32("-ratio");
	  	int order = cmd_ln_int32("-order");
	  	string smooth = cmd_ln_str("-smooth");
	  	string prune = cmd_ln_str("-prune");
	  	double theta = cmd_ln_float32("-theta");
	  	string count_pattern = cmd_ln_str("-pattern");
	  	string ifile = cmd_ln_str("-ifile");
	  	string prefix = cmd_ln_str("-prefix");

	  	string input_file;

	  	if ((ifile.empty()) && !noalign) {
	  		E_FATAL("Input file not provided\n");
	  	} else {
	  		input_file = ifile;
	  	}
	  	if (prefix.empty()) {
	  		E_FATAL("Output file not provided\n");
	  	}
		if(gen_testset && !ifile.empty()) {
			cout << "Splitting dictionary: " << input_file << " into training and test set" << endl;
			split(input_file, prefix, ratio);
			input_file = prefix+".train";
	  	}
		
	  	if (!noalign) {
	  		cout << "Using dictionary: " << input_file << endl;
	  		align(input_file, prefix, seq1_del, seq2_del, seq1_max,
	  				seq2_max, seq_sep, s1s2_sep,
	  				eps, skip, seq1in_sep, seq2in_sep,
	  				s1s2_delim, iter);
	  	}

	  	train_model(eps, s1s2_sep, skip, order, smooth, prefix, seq_sep, prune, theta, count_pattern);

	  	return 0;
	  }
