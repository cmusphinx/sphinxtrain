#include <s3/gauden.h>

int
main(void)
{
    uint32 veclen[1] = { 3 };
    vector_t ***var = gauden_alloc_param(1, 1, 1, veclen);
    int rv = 0;

    var[0][0][0][0] = 0.0f;
    var[0][0][0][1] = -1.0e-7f;
    var[0][0][0][2] = 2.0f;
    gauden_floor_variance_array(var, 1, 1, 1, veclen,
                                GAUDEN_EVAL_VAR_FLOOR);

    if (var[0][0][0][0] != GAUDEN_EVAL_VAR_FLOOR
        || var[0][0][0][1] != GAUDEN_EVAL_VAR_FLOOR
        || var[0][0][0][2] != 2.0f)
        rv = 1;

    gauden_free_param(var);
    return rv;
}
