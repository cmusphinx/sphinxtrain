/*
 * utils.h
 *
 *  Created on: Jun 24, 2012
 *      Author: John Salatas <jsalatas@users.sourceforge.net>
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <vector>
#include <cstring>

using namespace std;

string convertInt(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

void split_string(string* input, vector<string>* tokens, string* delim, bool g2pmapping = false){
	size_t start = 0;
	size_t len = 0;
	size_t pos = 0;

	while (start < input->size()) {
		if (delim->empty()) {
			len = 1;
		} else {
			pos = input->find(*delim, start);
			if (pos != string::npos) {
				len = pos - start;
			} else {
				len = input->size() - start;
			}
		}
		tokens->push_back(input->substr(start, len));
		if (delim->empty()) {
			start = start + len;
		} else {
			start = start + len + delim->size();
		}
	}

	// In case of a g2pmapping check if we
	// have only two tokens (ie a grapheme and phoneme)
	if(g2pmapping && tokens->size() > 2) {
		// in case of n > 2 tokens merge the first n-1 as token[0]
		// and keep token[n] as token[1]
		if(tokens->at(0) == "") {
			tokens->at(0) = *delim;
		}
		for(int i=1; i<tokens->size()-1; i++) {
			if(tokens->at(i) == "") {
				tokens->at(i) = *delim;
			}
			tokens->at(0).append(tokens->at(i));
		}
		tokens->at(0) = tokens->at(0).substr(0, tokens->at(0).size()-1);
		tokens->at(1) = tokens->at(tokens->size()-1);
	}
}


#endif /* UTILS_H_ */
