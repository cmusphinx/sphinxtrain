#ifndef G2P_TRAIN_H_
#define G2P_TRAIN_H_
#include <string>

using namespace std;

void split(string input_file, string prefix, int ratio);

void align(string input_file, string prefix, bool seq1_del, bool seq2_del, 
		int seq1_max, int seq2_max, string seq_sep, string s1s2_sep,
		string eps, string skip, string seq1in_sep, string seq2in_sep,
		string s1s2_delim, int iter);
		
void train_model(string eps, string s1s2_sep, string skip, int order, 
		string smooth, string prefix, string seq_sep, string prune, 
		double theta, string count_pattern);

#endif /* G2P_TRAIN_H_ */
