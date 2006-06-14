#include "dlamch.c"

/* ======================================================================= */

/* The pow_di function was copied from f2c_lite.c */
#ifdef KR_headers
double pow_di(ap, bp) doublereal *ap; integer *bp;
#else
double pow_di(doublereal *ap, integer *bp)
#endif
{
double pow, x;
integer n;
unsigned long u;

pow = 1;
x = *ap;
n = *bp;

if(n != 0)
        {
        if(n < 0)
                {
                n = -n;
                x = 1/x;
                }
        for(u = n; ; )
                {
                if(u & 01)
                        pow *= x;
                if(u >>= 1)
                        x *= x;
                else
                        break;
                }
        }
return(pow);
}

/* ======================================================================= */

/* The lsame function was copied from blas_lite.c */
logical lsame_(char *ca, char *cb)
{
/*  -- LAPACK auxiliary routine (version 3.0) --
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
       Courant Institute, Argonne National Lab, and Rice University
       September 30, 1994


    Purpose
    =======

    LSAME returns .TRUE. if CA is the same letter as CB regardless of
    case.

    Arguments
    =========

    CA      (input) CHARACTER*1
    CB      (input) CHARACTER*1
            CA and CB specify the single characters to be compared.

   =====================================================================



       Test if the characters are equal */
    /* System generated locals */
    logical ret_val;
    /* Local variables */
    static integer inta, intb, zcode;


    ret_val = *(unsigned char *)ca == *(unsigned char *)cb;
    if (ret_val) {
        return ret_val;
    }

/*     Now test for equivalence if both characters are alphabetic. */

    zcode = 'Z';

/*     Use 'Z' rather than 'A' so that ASCII can be detected on Prime
       machines, on which ICHAR returns a value with bit 8 set.
       ICHAR('A') on Prime machines returns 193 which is the same as
       ICHAR('A') on an EBCDIC machine. */

    inta = *(unsigned char *)ca;
    intb = *(unsigned char *)cb;

    if (zcode == 90 || zcode == 122) {

/*        ASCII is assumed - ZCODE is the ASCII code of either lower o
r
          upper case 'Z'. */

        if (inta >= 97 && inta <= 122) {
            inta += -32;
        }
        if (intb >= 97 && intb <= 122) {
            intb += -32;
        }

    } else if (zcode == 233 || zcode == 169) {

/*        EBCDIC is assumed - ZCODE is the EBCDIC code of either lower
 or
          upper case 'Z'. */

        if (inta >= 129 && inta <= 137 || inta >= 145 && inta <= 153 || inta
                >= 162 && inta <= 169) {
            inta += 64;
        }
        if (intb >= 129 && intb <= 137 || intb >= 145 && intb <= 153 || intb
                >= 162 && intb <= 169) {
            intb += 64;
        }

    } else if (zcode == 218 || zcode == 250) {

/*        ASCII is assumed, on Prime machines - ZCODE is the ASCII cod
e
          plus 128 of either lower or upper case 'Z'. */

        if (inta >= 225 && inta <= 250) {
            inta += -32;
        }
        if (intb >= 225 && intb <= 250) {
            intb += -32;
        }
    }
    ret_val = inta == intb;

/*     RETURN

       End of LSAME */

    return ret_val;
} /* lsame_ */

/* ======================================================================= */

int main(void)
{ double x;
  FILE* outputfile = fopen("lapack_config.h","w");
  x = dlamch_("E");
  fprintf(outputfile,"#define EPSILON %17.14g\n", x);
  x = dlamch_("S");
  fprintf(outputfile,"#define SAFEMINIMUM %17.14g\n", x);
  x = dlamch_("P");
  fprintf(outputfile,"#define PRECISION %17.14g\n", x);
  x = dlamch_("B");
  fprintf(outputfile,"#define BASE %f\n", x);
  fclose(outputfile);
  return 0;
}
  
