/* Include file used by everybody */

#define FIND_INDEX		find_word_phone_index
#define CEP_EXT			"cep"
#define FEAT_EXT		"feat"
#define LOLA_EXT		"ptlola"
#define CODE_EXT		"code"
#define WINDOW_WIDTH		20
#define ANALYSIS_STEP		10
#define LOLA_FRAME	 	3
#define MAX_CODEBOOKS		256
#define MAX_FRAMES		256
#define MIN_DOUBLE		1.0e-300
#define PHONE_FILE		"/usr/kfl/phrec/ctl/word.phone"
#define MAX_PHONE_STRING	30

struct phone
{
  char name[MAX_PHONE_STRING];
  short word_index, pinw, real_phone, output_phone;
};

#define NUM_CI	64

struct diphone {
  char name[30];
  short ci;
  short table[NUM_CI];
};

extern struct diphone *DiPhTable;
extern int Num_Di;
