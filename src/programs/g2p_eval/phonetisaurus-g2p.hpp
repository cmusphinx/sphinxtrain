#include <string>

using namespace std;

void phoneticizeWord(const char *g2pmodel_file, const char *output,
                     string testword, int nbest, string sep, int beam =
                         500, int output_words = 0);
void phoneticizeTestSet(const char *g2pmodel_file, const char *output,
                        string testset_file, int nbest, string sep,
                        int beam = 500, int output_words =
                            0, bool output_cost = true);
