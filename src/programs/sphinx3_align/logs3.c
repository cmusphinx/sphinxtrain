/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * logs3.c -- log(base-S3) module.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.10  2006/04/05  20:27:32  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.9  2006/03/03 19:45:00  egouvea
 * Clean up the log handling. In logs3.c, removed unnecessary variables
 * (e.g. "f", exactly the same as "F") and functions (e.g. "logs3_10base()").
 *
 * In confidence.c, replace (logs3_to_log10(r_lscr) * logs3_10base())
 * with r_lscr, since the only difference is that one is a double, the
 * other an int (and as such, they differ on the order of 1e-12).
 *
 * In future cleanups.... replace the "int" declaration with "int32",
 * used in the rest of the code.
 *
 * Revision 1.8  2006/02/22 19:55:02  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII: Add function logs3_base and logs3_10base.
 *
 *
 * Revision 1.6.4.2  2006/01/16 19:51:19  arthchan2003
 * Added a function to convert Sphinx 3 log to log 10.
 *
 * Revision 1.6.4.1  2005/07/05 21:29:31  arthchan2003
 * 1, Merged from HEAD.
 *
 * Revision 1.7  2005/07/05 13:12:39  dhdfu
 * Add new arguments to logs3_init() in some tests, main_ep
 *
 * Revision 1.6  2005/06/21 20:46:54  arthchan2003
 * 1, Added a report flag in logs3_init, 2, Fixed doxygen documentation, 3, Add the $ keyword.
 *
 * Revision 1.6  2005/06/03 06:12:56  archan
 * 1, Simplify and unify all call of logs3_init, move warning when logbase > 1.1 into logs3.h.  2, Change arguments to require arguments in align and astar.
 *
 * Revision 1.5  2005/05/27 01:15:44  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.4  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 28-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added log_to_logs3_factor(), and logs3_to_p().
 * 
 * 05-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#include "logs3.h"

logmath_t*
logs3_init(float64 base, int32 bReport, int32 bLogTable)
{
    logmath_t *lmath = logmath_init(base, 0, bLogTable);
    if (bReport)
        logs3_report(lmath);
    return lmath;
}

int32
logs3(logmath_t* logmath, float64 p)
{
    if (p <= 0.0) {
        E_WARN("logs3 argument: %e; using S3_LOGPROB_ZERO\n", p);
        return S3_LOGPROB_ZERO;
    }
    return logmath_log(logmath, p);
}


void
logs3_report(logmath_t* logmath)
{
    if (logmath) {
	uint32 size, width, shift;

	logmath_get_table_shape(logmath, &size, &width, &shift);
	E_INFO_NOFN("Initialization of the log add table\n");
	E_INFO_NOFN("Log-Add table size = %d x %d >> %d\n", size, width, shift);
	E_INFO_NOFN("\n");
    }
}
