/* ====================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All rights 
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
/*********************************************************************
 *
 * File: parse_cmd_ln.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef PARSE_CMD_LN_H

int
parse_cmd_ln(int argc, char *argv[]);

#define PARSE_CMD_LN_H




#endif /* PARSE_CMD_LN_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2005/06/05  22:00:34  arthchan2003
 * Log. Rewriting QUICK_COUNT using SphinxTrain command line functions. Several changes.
 * 1, Removal of -B -t because they were actually not implemented.
 * 2, Add SphinxTrain's style command line, help string and example string.
 * 3, Safe-guarding a, invalid file names, b, user didn't specify SIL in the phone list.
 * 4, Change all quit to E_INFO, also delete obsolete quit.c.  Will change the windows setup respectively.
 * 5, Fix bug 1151880.  That was caused by the use of magic phrase symbol &, the loop will read stuff out of memoery boundary  when & occurs at the end of the word.  This was fixed by specifically checking this particular condition in quick_phone.c.
 * 
 * Windows setup is not yet checked in. Will do right after the linux check-in.
 * 
 * Revision 1.2  2004/11/29 01:11:34  egouvea
 * Fixed license terms in some new files.
 *
 * Revision 1.1  2004/06/17 19:39:48  arthchan2003
 * add back all command line information into the code
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:38:16  eht
 * Initial revision
 * 
 * Revision 1.1  1995/12/15  18:38:27  eht
 * Initial revision
 *
 *
 */
