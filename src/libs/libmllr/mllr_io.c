#include <string.h>
#include <math.h>
#include <s3/s3.h>
#include <s3/common.h>
#include <s3/mllr.h>
#include <s3/mllr_io.h>

int32
store_reg_mat (const char    *regmatfn,
	       const uint32  *veclen,
	       uint32  n_class,
	       uint32  n_stream,
	       float32 ****A,
	       float32 ***B)
{
    uint32 i,j,k,m;
    FILE  *fp;

    if ((fp = fopen(regmatfn,"w")) == NULL) {
	E_INFO("Unable to open %s to store MLLR matrices\n",regmatfn);
	return S3_ERROR;
    }

    fprintf(fp,"%d\n",n_class);
    fprintf(fp,"%d\n",n_stream); 
    for (m = 0; m < n_class; m++) {
	for (i = 0; i < n_stream; i++) {
	    fprintf(fp,"%d\n", veclen[i]); 
	    for (j = 0; j < veclen[i]; j++) {
		for (k = 0; k < veclen[i]; ++k) {
		    fprintf(fp,"%f ",A[m][i][j][k]);
		}
		fprintf(fp,"\n");
	    }
	    for (j = 0; j < veclen[i]; j++) {
		fprintf(fp,"%f ",B[m][i][j]);
	    }
	    fprintf(fp,"\n");
	}
    }
    fclose(fp);
    return S3_SUCCESS;
}


int32
read_reg_mat (
             const char   *regmatfn,
             const uint32  **veclen,
             uint32  *n_class,
             uint32  *n_stream,
             float32 *****A,
             float32 ****B
            )
{
    uint32 i,j,k,m,nstream,nclass;
    FILE  *fp;
    uint32 *vlen;
    float32 ****lA,***lB;

    if ((fp = fopen(regmatfn,"r")) == NULL) {
	E_INFO("Unable to open %s to read MLLR matrices\n",regmatfn);
	return S3_ERROR;
    }

    fscanf(fp,"%d",&nclass);
    fscanf(fp,"%d",&nstream);
    vlen = (uint32 *)ckd_calloc(nstream,sizeof(uint32));
    lA = (float32 ****)ckd_calloc_2d (nclass,nstream,sizeof (float32 **));
    lB = (float32 ***)ckd_calloc_2d (nclass,nstream,sizeof (float32 *));
    for (m = 0; m < nclass; ++m) {
	for (i = 0; i < nstream; ++i) {
	    fscanf(fp,"%d", &vlen[i]);
	    lA[m][i] = (float32 **) ckd_calloc_2d(vlen[i],vlen[i],sizeof(float32));
	    lB[m][i] = (float32 *) ckd_calloc(vlen[i],sizeof(float32));
	    for (j = 0; j < vlen[i]; j++) {
		for (k = 0; k < vlen[i]; ++k) {
		    fscanf(fp,"%f ",&lA[m][i][j][k]);
		}
	    }
	    for (j = 0; j < vlen[i]; j++) {
		fscanf(fp,"%f ",&lB[m][i][j]);
	    }
	}
    }

    *n_class = nclass;
    *n_stream = nstream;
    *veclen = vlen;
    *A = lA;
    *B = lB;

    fclose(fp);
    return S3_SUCCESS;
}
