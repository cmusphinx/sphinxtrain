#include "phonetisaurus-g2p.h"
#include <sphinxbase/cmd_ln.h>


const char helpstr[] =
    "Usage: g2p_eval -model MODEL -input INPUT [-output OUTPUT] [-isfile] [-output_cost] \n\
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

    const char * model = cmd_ln_str("-model");
    const char * input = cmd_ln_str("-input");
    const char * output = cmd_ln_str("-output");
    int output_cost = cmd_ln_boolean("-output_cost");
    int isfile = cmd_ln_boolean("-isfile");
    int nbest = cmd_ln_int32("-nbest");
    int beam = cmd_ln_int32("-beam");
    const char * sep = cmd_ln_str("-sep");
    int words = cmd_ln_boolean("-words");

    if (isfile) {
        phoneticizeTestSet(model, output, input, nbest,
                           sep, beam, words, output_cost);
    }
    else {
        phoneticizeWord(model, output, input, nbest, sep,
                        beam, words, output_cost);
    }

    return 0;
}
