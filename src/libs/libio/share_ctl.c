/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * File: share_ctl.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/prim_type.h>
#include <s3/swap.h>
#include <s3/bo_magic.h>
#include <s3/s3.h>

#include <sys_compat/file.h>

static uint32 blk_sz;
static uint32 buf_id;
static int state_fd;

int
share_ctl_set_block_size(uint32 sz)
{
    blk_sz = sz;
    return S3_SUCCESS;
}

int
share_ctl_set_buf_id(uint32 id)
{
    buf_id = id;

    return S3_SUCCESS;
}

int
share_ctl_set_state_file(const char *file_name)
{
    state_fd = open(file_name, O_CREAT | O_RDWR, 0755);
    if (state_fd < 0) {
	E_FATAL_SYSTEM("Couldn't open ctl file state file %s\n",
		       file_name);
    }

    return S3_SUCCESS;
}

int
share_ctl_init_state_file()
{
    uint32 magic = BYTE_ORDER_MAGIC;
    uint32 zero = 0;

    if (write(state_fd, &magic, sizeof(uint32)) < 0)
	E_FATAL_SYSTEM("write failed");
    
    if (write(state_fd, &zero, sizeof(uint32)) < 0)
	E_FATAL_SYSTEM("write failed");

    lseek(state_fd, (off_t)0, SEEK_SET);
    return S3_SUCCESS;
}

int
share_ctl_next_utts(uint32 *offset, uint32 *run)
{
    uint32 magic;
    uint32 i_offset;
    uint32 o_offset;
    uint32 ns;
    int ret;
    struct flock lck;

    lck.l_type = F_WRLCK;
    lck.l_whence = SEEK_SET;
    lck.l_start = 0;
    lck.l_len = 2 * sizeof(uint32);

    /* Get an exclusive lock on the file */
    if (fcntl(state_fd, F_SETLKW, &lck) < 0) {
	E_FATAL_SYSTEM("file lock failed");
    }

    printf("*** in crit sec ***\n");

    /* read the byte order magic number */
    ret = read(state_fd, &magic, sizeof(uint32));
    if (ret < 0)
	E_FATAL_SYSTEM("read failed");
    else if (ret < sizeof(uint32)) {
	E_FATAL("Expected to read %u bytes, but got %u instead\n",
		sizeof(uint32), ret);
    }
	       

    if (magic != BYTE_ORDER_MAGIC) {
	/* seems to need swap */

	SWAP_INT32(&magic);

	if (magic != BYTE_ORDER_MAGIC) {
	    E_FATAL("Couldn't create magic # by swapping\n");
	}

	ns = TRUE;
    }
    else
	ns = FALSE;


    ret = read(state_fd, &i_offset, sizeof(uint32));
    if (ret < 0)
	E_FATAL_SYSTEM("read failed");
    else if (ret < sizeof(uint32)) {
	E_FATAL("Expected to read %u bytes, but got %u instead\n",
		sizeof(uint32), ret);
    }
    
    if (ns)
	SWAP_INT32(&i_offset);

    o_offset = i_offset + blk_sz;

    printf("i_offset= %u\n", i_offset);

    sleep(30);

    lseek(state_fd, (off_t)0, SEEK_SET);

    if (write(state_fd, &magic, sizeof(uint32)) < 0)
	E_FATAL_SYSTEM("write failed");
    
    if (write(state_fd, &o_offset, sizeof(uint32)) < 0)
	E_FATAL_SYSTEM("write failed");
    
    lck.l_type = F_UNLCK;

    if (fcntl(state_fd, F_SETLK, &lck) < 0) {
	E_FATAL_SYSTEM("file unlock failed");
    }

    lseek(state_fd, (off_t)0, SEEK_SET);

    printf("*** out crit sec ***\n");

    *offset = i_offset;
    *run = blk_sz;

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:13  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
