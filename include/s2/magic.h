/* magic numbers defining types of HMM files.
 */

#define COUNT_F -1
#define PROB_F -2
#define OLD_FORMAT_F -3
#define COUNT3_F -4
#define PROB3_F -5
#define TIED_DIST	-10
#define BIG_HMM		-100

#define COUNT_P(_var) ((_var)==COUNT_F)
#define OLD_P(_var) ((_var)==OLD_FORMAT_F)

#define IS_MAGIC(_var) ((_var) < 0)

/* tags for kinds of data user wants */
#define BOTH 0
#define PROBS 1
#define COUNTS 2
#define EITHER 3
#define TOPOLOGY_ONLY 4

