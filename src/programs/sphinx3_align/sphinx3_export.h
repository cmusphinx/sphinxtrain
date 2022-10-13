#ifndef __S3DECODER_EXPORT_H__
#define __S3DECODER_EXPORT_H__

/* Win32/WinCE DLL gunk */
#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(__CYGWIN__)
#ifdef S3DECODER_EXPORTS
#define S3DECODER_EXPORT __declspec(dllexport)
#else
#define S3DECODER_EXPORT __declspec(dllimport)
#endif
#else /* !_WIN32 */
#ifdef HAVE_ATTRIBUTE_VISIBILITY
#define S3DECODER_EXPORT __attribute__ ((visibility("default")))
#else
#define S3DECODER_EXPORT
#endif
#endif

#if defined(__GNUC__) && __GNUC__ > 2
#define S3DECODER_DEPRECATED __attribute__ ((deprecated))
#else
#define S3DECODER_DEPRECATED
#endif

#endif /* __S3DECODER_EXPORT_H__ */
