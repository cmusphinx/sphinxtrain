#include <s3/prim_type.h>
#include <s3/s3.h>

#include <sys/time.h>
#include <sys/resource.h>

int
get_cpu_time(uint32 *sec, float32 *frac_sec)
{
#if defined(__alpha)
    int ret;
    struct rusage r_usage;
    uint32 out_sec;
    uint32 t_usec;
    uint32 out_usec;


    ret = getrusage(RUSAGE_SELF, &r_usage);
    if (ret < 0)
	return S3_ERROR;

    out_sec = 0;

    out_sec += r_usage.ru_utime.tv_sec;
    out_sec += r_usage.ru_stime.tv_sec;

    t_usec = r_usage.ru_utime.tv_usec+r_usage.ru_stime.tv_usec;

    out_sec += t_usec / 1000000;
    out_usec = t_usec % 1000000;

    *sec = out_sec;
    *frac_sec = (float32)out_usec / (float32)1000000;

    return S3_SUCCESS;
#else
    *sec = 0;
    *frac_sec = 0;

    return S3_SUCCESS;
#endif
}
