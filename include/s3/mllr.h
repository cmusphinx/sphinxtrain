#include <s3/feat.h>
#define TINY 1.0e-20;


int32 regmat_read (
	     const char    *accumdir,
             float32 ******regl,
             float32 *****regr,
             const uint32  **veclen,
             uint32  *n_mllr_class,
             uint32  *n_stream,
             uint32  *mllr_mult,
             uint32  *mllr_add
            );


int32 compute_mllr (
             float32 *****regl,
             float32 ****regr,
             const uint32  *veclen,
             uint32  nclass,
             uint32  nfeat,
             uint32  mllr_mult,
             uint32  mllr_add,
             float32 *****A,
             float32 ****B
            );


int32
invert(float32 **ainv,
       float32 **a,
       int32 len);

int32 solve  (
        float32 **regl,
        float32 *regr,
        int32   len,
        float64 *A
       );

int32 ludcmp (
         float64 **a,
         int32   n,
         int32   *indx,
         float64 *d
        );

int32 lubksb(
       float64 **a,
       int32   n,
       int32   *indx,
       float64 b[],
       float64 x[]
      );


