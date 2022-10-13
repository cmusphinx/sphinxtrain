#ifndef __S3DECODER_EXPORT_H__
#define __S3DECODER_EXPORT_H__

/* No DLL, no problem! */
#define S3DECODER_EXPORT

#if defined(__GNUC__) && __GNUC__ > 2
#define S3DECODER_DEPRECATED __attribute__ ((deprecated))
#else
#define S3DECODER_DEPRECATED
#endif

#endif /* __S3DECODER_EXPORT_H__ */
