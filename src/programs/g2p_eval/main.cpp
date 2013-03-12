#include <sphinxbase/cmd_ln.h>
#include "phonetisaurus-g2p.hpp"


using namespace std;

const char helpstr[] =
    "Usage: phonetisaurus-g2p -model MODEL -input INPUT [-output OUTPUT] [-isfile] [-output_cost] \n\
		               [-nbest NBEST] [-beam BEAM] [-sep SEP] [-words] \n\
		\n\
		-model MODEL,   The input WFST G2P model. \n\
		-input INPUT,   A word or test file. \n\
		-output OUTPUT, Output hypotheses to file. \n\
		-isfile,        INPUT is a file. Defaults to false. \n\
		-output_cost,   Output cost in the hyp file. Defaults to false. \n\
		-nbest NBEST,   Output the N-best pronunciations. Defaults to 1. \n\
		-beam BEAM,     N-best search beam. Defaults to 500. \n\
		-sep SEP,       Separator token for input words. Defaults to ''. \n\
		-words,         Output words with hypotheses. Defaults to false.";

int
main(int argc, char *argv[])
{
    static arg_t defn[] = {
        {"-help", ARG_BOOLEAN, "no", "Shows the usage of the tool"},
        {"-model", REQARG_STRING, "", "The input WFST G2P model."},
        {"-input", REQARG_STRING, "", "A word or test file."},
        {"-output", ARG_STRING, "", "Output hypotheses to file."},
        {   "-isfile", ARG_BOOLEAN, "no",
            "INPUT is a file. Defaults to false."
        },
        {   "-output_cost", ARG_BOOLEAN, "no",
            "Output cost in the hyp file. Defaults to false."
        },
        {   "-nbest", ARG_INT32, "1",
            "Output the N-best pronunciations. Defaults to 1."
        },
        {   "-beam", ARG_INT32, "500",
            "N-best search beam. Defaults to 500."
        },
        {   "-sep", ARG_STRING, "",
            "Separator token for input words. Defaults to ''."
        },
        {   "-words", ARG_BOOLEAN, "no",
            "Output words with hypotheses. Defaults to false."
        },
        {NULL, 0, NULL, NULL}
    };

    cmd_ln_parse(defn, argc, argv, TRUE);

    if (cmd_ln_int32("-help")) {
        printf("%s\n\n", helpstr);
    }

    string model = cmd_ln_str("-model");
    string input = cmd_ln_str("-input");
    string output = cmd_ln_str("-output");
    bool output_cost = cmd_ln_boolean("-output_cost");
    bool isfile = cmd_ln_boolean("-isfile");
    int nbest = cmd_ln_int32("-nbest");
    int beam = cmd_ln_int32("-beam");
    string sep = cmd_ln_str("-sep");
    bool words = cmd_ln_boolean("-words");

    if (isfile) {
        //If its a file, go for it
        phoneticizeTestSet(model.c_str(), output.c_str(), input, nbest,
                           sep, beam, words, output_cost);
    }
    else {
        //Otherwise we just have a word
        phoneticizeWord(model.c_str(), output.c_str(), input, nbest, sep,
                        beam, words);
    }

    return 0;
}
