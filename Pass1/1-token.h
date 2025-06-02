#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

#define LEN_SYMBOL 20

FILE *ASM_open(char *fname);
void ASM_close(void);
int ASM_token(char *buf);

#endif
