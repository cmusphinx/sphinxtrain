/* in place byte order conversion
   nothing is promised to be returned
   currently only works for suns and Vax MIPS machines
 */

/* defines needed for little endian machines */
#if defined(mips) || defined(__alpha) || defined(i386) || defined(WIN32)
#define NEEDS_SWAP

#define SWAPBYTES

#define SWAPW(x)	*(x) = ((0xff & (*(x))>>8) | (0xff00 & (*(x))<<8))
#define SWAPL(x)	*(x) = ((0xff & (*(x))>>24) | (0xff00 & (*(x))>>8) |\
			(0xff0000 & (*(x))<<8) | (0xff000000 & (*(x))<<24))
#define SWAPF(x)	SWAPL((int *) x)
#define SWAPP(x)	SWAPL((int *) x)
#define SWAPD(x)	{ int *low = (int *) (x), *high = (int *) (x) + 1,\
			      temp;\
			  SWAPL(low);  SWAPL(high);\
			  temp = *low; *low = *high; *high = temp;}

#else	/* don't need byte order conversion, do nothing */

#undef NEEDS_SWAP

#define SWAPW(x)
#define SWAPL(x)
#define SWAPF(x)
#define SWAPP(x)
#define SWAPD(x)

#endif


