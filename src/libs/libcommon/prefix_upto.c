#include <s3/prefix_upto.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

char *prefix_upto(char **in_str, char delimiter)
{
    static char big_str[1024];
    char *in;
    char *delm;
    int prefix_len;

    assert(in_str != NULL);

    in = *in_str;

    delm = strchr(in, delimiter);
    if (delm == NULL) {
	return NULL;
    }

    prefix_len = delm - in;

    assert(prefix_len < 1024);

    strncpy(big_str, in, prefix_len);

    big_str[prefix_len] = '\0';

    *in_str = delm + 1;

    return big_str;
}
