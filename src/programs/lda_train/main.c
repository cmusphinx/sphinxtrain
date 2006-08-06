/* -*- c-basic-offset:4; indent-tabs-mode: nil -*- */

#include "parse_cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/ckd_alloc.h>
#include <s3/feat.h>
#include <s3/s3io.h>
#include <s3/matrix.h>
#include <s3/clapack_lite.h>
#include <s3/vector.h>
#include <s3/corpus.h>
#include <s3/model_def_io.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <string.h>

#define MATRIX_FILE_VERSION "0.1"

static int
lda_save(const char *outfile, float32 ***lda, int32 nlda, int32 featlen)
{
        FILE *outfh;
        uint32 val, chksum = 0;

        s3clr_fattr();
        s3add_fattr("version", MATRIX_FILE_VERSION, TRUE);
        s3add_fattr("chksum0", "yes", TRUE);
        if ((outfh = s3open(cmd_ln_access("-outfn"), "wb", NULL)) == NULL) {
            E_FATAL_SYSTEM("Failed to open %s for writing", cmd_ln_access("-outfn"));
        }
        s3write_3d((void ***)lda, sizeof(float32), nlda, featlen, featlen, outfh, &chksum);
        s3write(&chksum, sizeof(chksum), 1, outfh, &val);
        s3close(outfh);
        return 0;
}

static void
calc_scatter(float32 ***out_sw, float32 ***out_sb, uint32 *out_featlen)
{
    uint32 tick_cnt;
    uint16 *seg;
    vector_t *mfcc;
    vector_t **feat;
    uint32 *class_n_frame;
    uint32 n_class, n_frame, i;
    model_def_t *mdef;
    uint32 ceplen, featlen;
    float32 *globalmean, **mean, ***scatter, **sw, **sb, **op;

    /* Set up corpus and mdef. */
    if (model_def_read(&mdef,
                       cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
        E_FATAL_SYSTEM("Unable to open model def file");
    }

    corpus_set_ctl_filename(cmd_ln_access("-ctlfn"));
    corpus_set_seg_dir(cmd_ln_access("-segdir"));
    corpus_set_seg_ext(cmd_ln_access("-segext"));
    corpus_set_mfcc_dir(cmd_ln_access("-cepdir"));
    corpus_set_mfcc_ext(cmd_ln_access("-cepext"));
    if (corpus_init() != S3_SUCCESS) {
        E_FATAL("Corpus initialization failed\n");
    }

    ceplen = cmd_ln_int32("-ceplen");
    feat_set(cmd_ln_access("-feat"));
    feat_set_in_veclen(ceplen);
    if (feat_n_stream() > 1) {
        E_FATAL("Multi-stream features are not supported\n");
    }
    featlen = feat_blksize();

    /* Use all non-filler senones as classes. */
    n_class = mdef->n_tied_state;

    /* Per-class frame count */
    class_n_frame = ckd_calloc(n_class, sizeof(uint32));
    /* Per-class mean */
    mean = (float32 **)ckd_calloc_2d(n_class, featlen, sizeof(float32));
    /* Global mean */
    globalmean = ckd_calloc(featlen, sizeof(float32));
    /* Per-class covariance (I think) */
    scatter = (float32 ***)ckd_calloc_3d(n_class, featlen, featlen, sizeof(float32));
    /* Within-class covariance */
    sw = (float32 **)ckd_calloc_2d(featlen, featlen, sizeof(float32));
    /* Between-class covariance */
    sb = (float32 **)ckd_calloc_2d(featlen, featlen, sizeof(float32));
    /* Temporary accumulator for outer-product */
    op = (float32 **)ckd_calloc_2d(featlen, featlen, sizeof(float32));

    /* Accumulate for means */
    E_INFO("Accumulating for means...\n");
    tick_cnt = 0;
    n_frame = 0;
    while (corpus_next_utt()) {
        uint32 utt_n_frame, tmp_n_frame;

	if ((++tick_cnt % 500) == 0) {
	    printf("[%u] ", tick_cnt);
	    fflush(stdout);
	}
	if (corpus_get_seg(&seg, &utt_n_frame) != S3_SUCCESS) {
	    E_FATAL("Unable to read Viterbi state segmentation for %s\n", corpus_utt_brief_name());
	}
        corpus_get_generic_featurevec(&mfcc, &tmp_n_frame, ceplen);
        if (tmp_n_frame != utt_n_frame) {
            E_ERROR("State segmentation/control file mismatch in #frames: %u != %u\n",
                    utt_n_frame, tmp_n_frame);
            ckd_free(mfcc[0]);
            ckd_free(mfcc);
            mfcc = NULL;
            ckd_free(seg);
            seg = NULL;
            continue;
        }
        feat = feat_compute(mfcc, &utt_n_frame);

        for (i = 0; i < utt_n_frame; ++i) {
            uint32 class = seg[i] & 0x7fff; /* We do not care if it's the first state */

            vector_add(mean[class], feat[i][0], featlen);
            vector_add(globalmean, feat[i][0], featlen);
            ++class_n_frame[class];
        }
        n_frame += utt_n_frame;

        feat_free(feat);
        ckd_free(mfcc[0]);
        ckd_free(mfcc);
    }
    printf("\n");
    /* Normalize for class and global means */
    for (i = 0; i < n_class; ++i) {
        if (class_n_frame[i] != 0.0) {
            vector_scale(mean[i], 1.0/class_n_frame[i], featlen);
        }
    }
    if (n_frame != 0.0)
        vector_scale(globalmean, 1.0/n_frame, featlen);

    /* Accumulate for covariances */
    corpus_reset();
    E_INFO("Accumulating for covariances...\n");
    tick_cnt = 0;
    while (corpus_next_utt()) {
	if ((++tick_cnt % 500) == 0) {
	    printf("[%u] ", tick_cnt);
	    fflush(stdout);
	}
	corpus_get_seg(&seg, &n_frame);
        corpus_get_generic_featurevec(&mfcc, &n_frame, ceplen);
        feat = feat_compute(mfcc, &n_frame);

        for (i = 0; i < n_frame; ++i) {
            uint32 class = seg[i] & 0x7fff;

            vector_sub(feat[i][0], mean[class], featlen);
            outerproduct(op, feat[i][0], feat[i][0], featlen);
            matrixadd(scatter[class], op, featlen, featlen);
        }

        feat_free(feat);
        ckd_free(mfcc[0]);
        ckd_free(mfcc);
    }
    printf("\n");

    /* Sw = sum_i p_i scatter_i */
    for (i = 0; i < n_class; ++i) {
        /* Normalize scatter_i:
           (class_n_frame[i]/n_frame) * (1/class_n_frame[i]) = 1/n_frame */
        scalarmultiply(scatter[i], 1.0/n_frame, featlen, featlen);
        matrixadd(sw, scatter[i], featlen, featlen);
    }

    /* Sb = sum_i (mean_i - globalmean) (mean_i - globalmean)^T */
    for (i = 0; i < n_class; ++i) {
        vector_print(mean[i], featlen);
        vector_sub(mean[i], globalmean, featlen);
        outerproduct(op, mean[i], mean[i], featlen);
        matrixadd(sb, op, featlen, featlen);
    }
    ckd_free_3d((void ***)scatter);
    ckd_free_2d((void **)mean);
    ckd_free_2d((void **)op);
    ckd_free(globalmean);

    if (out_sw) *out_sw = sw;
    if (out_sb) *out_sb = sb;
    if (out_featlen) *out_featlen = featlen;
}

static int eigen_sort(const void *a, const void *b)
{
    const float32 *aa = a, *bb = b;

    /* Sort descending. */
    if (bb[0] > aa[0])
        return -1;
    else if (bb[0] < aa[0])
        return 1;
    else
        return 0;
}

static void
lda_compute(float32 **sw, float32 **sb,
            float32 **out_u, float32 ***out_v,
            uint32 featlen)
{
    float32 **swinv, **ba, *ur, *ui, **vr, **vi, **eigen;
    uint32 i, j;

#if 0
    printf("Sw:\n");
    for (i = 0; i < featlen; ++i) {
        for (j = 0; j < featlen; ++j) {
            printf("%f ", sw[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Sb:\n");
    for (i = 0; i < featlen; ++i) {
        for (j = 0; j < featlen; ++j) {
            printf("%f ", sb[i][j]);
        }
        printf("\n");
    }
    printf("\n");
#endif

    /* Solve the eigenproblem B^{-1}Av = uv (this isn't the most
     * efficient way to do this but the matrix is pretty small so it
     * doesn't matter) */
    swinv = (float32 **)ckd_calloc_2d(featlen, featlen, sizeof(float32));
    if (invert(swinv, sw, featlen) < 0) {
        E_FATAL("Singular Sw matrix!! Argh!\n");
    }
    ba = (float32 **)ckd_calloc_2d(featlen, featlen, sizeof(float32));
    vr = (float32 **)ckd_calloc_2d(featlen, featlen, sizeof(float32));
    vi = (float32 **)ckd_calloc_2d(featlen, featlen, sizeof(float32));
    ur = ckd_calloc(featlen, sizeof(float32));
    ui = ckd_calloc(featlen, sizeof(float32));

    matrixmultiply(ba, swinv, sb, featlen, featlen, featlen);
    eigenvectors(ba, ur, ui, vr, vi, featlen);

    /* Discard the imaginary parts (they will be zero because B^{-1}A
     * is symmetric).  Yes, we could have used SSYGV... */
    ckd_free_2d((void **)vi);
    ckd_free(ui);

    /* Construct the LDA transformation. LAPACK is supposed to return
     * the eigenvalues in sorted order but that doesn't actually
     * always happen, so we will sort them to be sure here.  */
    /* Concatenate eigenvalues/eigenvectors. */
    eigen = (float32 **)ckd_calloc_2d(featlen, featlen+1, sizeof(float32));
    for (i = 0; i < featlen; ++i) {
        eigen[i][0] = ur[i];
        memcpy(&eigen[i][1], vr[i], featlen * sizeof(float32));
    }
    qsort(eigen, featlen, sizeof(float32 *), eigen_sort);
    /* Copy the eigenvalues/vectors back to ur, vr */
    for (i = 0; i < featlen; ++i) {
        ur[i] = eigen[i][0];
        memcpy(vr[i], &eigen[i][1], featlen * sizeof(float32));
    }
    
    for (i = 0; i < featlen; ++i) {
        printf("Eigenvalue %d: %f\n", i, eigen[i][0]);
        printf("Eigenvector %d: \n", i);
        for (j = 0; j < featlen; ++j) {
            printf("%f ", eigen[i][j+1]);
        }
        printf("\n");
    }

    ckd_free_2d((void **)eigen);
    if (out_u) *out_u = ur;
    if (out_v) *out_v = vr;
}

int
main(int argc, char *argv[])
{
    float32 **sw, **sb, **lda;
    uint32 featlen;

    /* Get arguments. */
    parse_cmd_ln(argc, argv);

    /* Check required arguments (imagine that) */
    if (cmd_ln_access("-ctlfn") == NULL
        || cmd_ln_access("-cepdir") == NULL
        || cmd_ln_access("-segdir") == NULL
        || cmd_ln_access("-moddeffn") == NULL
        || cmd_ln_access("-feat") == NULL) {
        E_FATAL("Some required arguments missing (-ctlfn, -cepdir, -segdir, -moddeffn, -feat)\n");
    }

    /* Calculate scatter matrices */
    calc_scatter(&sw, &sb, &featlen);

    /* Do eigen decomposition to find LDA matrix. */
    lda_compute(sw, sb, NULL, &lda, featlen);

    /* Write out the matrix. */
    if (cmd_ln_access("-outfn") != NULL) {
        lda_save(cmd_ln_access("-outfn"), &lda, 1, featlen);
    }

    ckd_free_2d((void **)sw);
    ckd_free_2d((void **)sb);
    ckd_free_2d((void **)lda);
    return 0;
}
