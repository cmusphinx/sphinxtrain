/* ====================================================================
 * Copyright (c) 1998-2000 Carnegie Mellon University.  All rights 
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
 * File: cmd_ln_defn.h
 * 
 * Description: 
 * 	Command line definitions for bw
 *
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef CMD_LN_DEFN_H
#define CMD_LN_DEFN_H


    const char helpstr[] =  
"Description: \n\
\n\
    A routine that provides and ad-hoc way of speaker adaptation by mixture \n\
    weight interpolation.  SD and SI model's mixture weight are first \n\
    determined and they act as an inputs of this program. The output \n\
    is the interpolated mixture weight. \n\
\n\
    The interpolation is controlled by the value lambda (-sillambda) ";

    const char examplestr[]=
"Example: \n\
\n\
    mixw_interp -SImixwfn si_mixw -SDmixwfn  sd_mxiw -outmixwfn final_mixw -SIlambad 0.7";
    static arg_def_t defn[] = {
	{ "-help",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Shows example of how to use the tool"},

        { "-SImixwfn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "The SI mixture weight parameter file name"},

        { "-SDmixwfn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "The SD mixture weight parameter file name"},

        { "-outmixwfn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "The output interpolated mixture weight parameter file name"},

        { "-SIlambda",
          CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
          "0.5",
          "Weight given to SI mixing weights" },

	{ NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
    };


#endif /* CMD_LN_DEFN_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.5  2004/08/10  21:58:51  arthchan2003
 * Incorporate help and example for the four final tools
 * 
 * Revision 1.4  2004/07/21 18:30:35  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
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
 *
 */
