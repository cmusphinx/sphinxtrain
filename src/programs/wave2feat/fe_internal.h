

#ifndef	M_PI
#define M_PI	(3.14159265358979323846)
#endif	M_PI

#define FORWARD_FFT 1
#define INVERSE_FFT -1

typedef struct { float64 r, i; } complex;

/* functions */
int32 fe_build_melfilters(melfb_t *MEL_FB);
int32 fe_compute_melcosine(melfb_t *MEL_FB);
float32 fe_mel(float32 x);
float32 fe_melinv(float32 x);
void fe_pre_emphasis(int16 *in, float64 *out, int32 len, float32 factor, int16 prior);
void fe_create_hamming(float64 *in, int32 in_len);
void fe_hamming_window(float64 *in, float64 *window, int32 in_len);
void fe_init_hamming(float64 *win, int32 len);
void fe_spec_magnitude(float64 *data, int32 data_len, float64 *spec, int32 fftsize);
void fe_frame_to_fea(fe_t *FE, float64 *in, float64 *fea);
void fe_mel_spec(fe_t *FE, float64 *spec, float64 *mfspec);
void fe_mel_cep(fe_t *FE, float64 *mfspec, float64 *mfcep);
int32 fe_fft(complex *in, complex *out, int32 N, int32 invert);
void fe_short_to_double(int16 *in, float64 *out, int32 len);
char **fe_create_2d(int32 d1, int32 d2, int32 elem_size);
void fe_free_2d(void **arr);
void fe_print_current(fe_t *FE);
void fe_parse_general_params(param_t *P, fe_t *FE);
void fe_parse_melfb_params(param_t *P, melfb_t *MEL);




