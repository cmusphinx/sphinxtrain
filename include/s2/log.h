/* last edit by Joe Keane on October 10, 1989 */

#define BASE		1.0001
#define LOG_BASE	9.9995000333297321e-05
#define R_LOG_BASE	1.0000499991668185e+04
/* #define MIN_LOG		(-1 << 29) */
#define MIN_LOG		(-1 << 27)
#define ADDITION_TABLE_SIZE 99042
#define SUBTRACTION_TABLE_SIZE 99042

#define FIX(X)		((X) <= MIN_LOG ? MIN_LOG : (X))
#define EXP(X)		(exp ((X) * LOG_BASE))
#define LOG(X)		((X) <= 0 ? MIN_LOG : (int)  (log ((double) (X)) * R_LOG_BASE))
#define MULTIPLY(X, Y)	((X) + (Y) <= MIN_LOG ? MIN_LOG : (X) + (Y))
#define ADD(X, Y)	((X) > (Y) ? (Y) <= MIN_LOG || (unsigned) ((X) - (Y)) >= ADDITION_TABLE_SIZE ? (X) : (X) + Addition_Table[(X) - (Y)] : (X) <= MIN_LOG || (unsigned) ((Y) - (X)) >= ADDITION_TABLE_SIZE ? (Y) : (Y) + Addition_Table[(Y) - (X)])
#define SUBTRACT(X, Y)	((X) <= (Y) ? MIN_LOG : (unsigned) ((X) - (Y)) >= SUBTRACTION_TABLE_SIZE ? (X) : (X) - Subtraction_Table[(X) - (Y)])

/* ADD_ASSIGN (X, Y) is equivalent to X = ADD (X, Y) and MULTIPLY_ASSIGN (X, Y) is equivalent to X = MULTIPLY (X, Y) but they are faster */
#define MULTIPLY_ASSIGN(X, Y) ((X) + (Y) <= MIN_LOG ? ((X) = MIN_LOG) : ((X) += (Y)))
#define ADD_ASSIGN(X, Y) ((X) > (Y) ? (Y) <= MIN_LOG || (unsigned) ((X) - (Y)) >= ADDITION_TABLE_SIZE ? (X) : ((X) += Addition_Table[(X) - (Y)]) : (X) <= MIN_LOG || (unsigned) ((Y) - (X)) >= ADDITION_TABLE_SIZE ? ((X) = (Y)) : ((X) = (Y) + Addition_Table[(Y) - (X)]))
#define SUBTRACT_ASSIGN(X, Y) ((X) <= (Y) ? ((X) = MIN_LOG) : (unsigned) ((X) - (Y)) >= SUBTRACTION_TABLE_SIZE ? (X) : ((X) -= Subtraction_Table[(X) - (Y)]))

#include <math.h>

extern short Addition_Table[ADDITION_TABLE_SIZE];
extern int Subtraction_Table[SUBTRACTION_TABLE_SIZE];
