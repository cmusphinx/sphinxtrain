/* LIST.H
 *------------------------------------------------------------*
 */

#ifndef _LIST_H_
#define _LIST_H_

#include <s3/prim_type.h>

#include <sys/types.h>

typedef struct {
	int32	size;			/* Number entries in the list */
	int32	in_use;			/* Number entries in use in list */
	caddr_t	*list;			/* The list */
} list_t;

extern list_t *new_list(void);


#endif
