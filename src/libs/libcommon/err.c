/*
 * Package for checking and catching common errors, printing out
 * errors nicely, etc.
 *
 * 6/01/95  Paul Placeway  CMU speech group
 *
 * 6/02/95  Eric Thayer
 *	- Removed non-ANSI expresssions.  I don't know of any non-ANSI
 *		holdouts left anymore. (DEC using -std1, HP using -Aa,
 *		Sun using gcc or acc.)
 *      - Removed the automatic newline at the end of the error message
 *	  as that all S3 error messages have one in them now.
 *	- Added an error message option that does a perror() call.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <s3/err.h>

void
_E__pr_header(const char *f, long ln, const char *msg)
{
    (void) fflush(stdout);
    (void) fprintf(stderr, "%s: \"%s\", line %ld: ", msg, f, ln);
}

void
_E__pr_info_header(const char *f, long ln, const char *msg)
{
    (void) fflush(stdout);

    /* make different format so as not to be parsed by emacs compile */
    (void) fprintf(stderr, "%s: %s(%ld): ", msg, f, ln);
}

void
_E__pr_warn( char *fmt, ... ) 
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    (void) fflush(stderr);
}

void
_E__pr_info( char *fmt, ... ) 
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    (void) fflush(stdout);
}

void _E__die_error( char *fmt, ... ) 
{
    va_list pvar;

    va_start(pvar, fmt);

    (void) vfprintf(stderr, fmt, pvar);
    (void) fflush(stderr);

    va_end(pvar);

    (void) fflush(stderr);
    
    exit (-1);
}

void _E__fatal_sys_error( char *fmt, ... ) 
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    putc(';', stderr);
    putc(' ', stderr);

    perror("");
    
    (void) fflush(stderr);

    exit(errno);
}

void _E__sys_error( char *fmt, ... ) 
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    putc(';', stderr);
    putc(' ', stderr);

    perror("");

    (void) fflush(stderr);
}

void _E__abort_error( char *fmt, ... ) 
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    (void) fflush(stderr);

    abort ();
}

#ifdef TEST
main()
{
    char *two = "two";
    char *three = "three";
    FILE *fp;

    E_WARN("this is a simple test\n");

    E_WARN("this is a test with \"%s\" \"%s\".\n", "two", "arguments");

    E_WARN("foo %d is bar\n", 5);

    E_WARN("bar is foo\n");

    E_WARN("one\n", two, three);

    E_INFO("Just some information you might find interesting\n");

    fp = fopen("gondwanaland", "r");
    if (fp == NULL) {
	E_SYSTEM("Can't open gondwanaland for reading");
    }
}
#endif /* TEST */
