#include "codec.h"

#ifdef __cplusplus
extern "C" {
#endif

int disas(struct instr_t instr, unsigned int offset, char *buffer);

int parse_param(const char *para, const char *para_def, struct instr_t *result);

int as(const char *str, unsigned int offset, struct instr_t *result);

#ifdef __cplusplus
}
#endif
