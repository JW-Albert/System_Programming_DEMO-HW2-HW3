#include <stdio.h>
#include <string.h>

#define LEN_SYMBOL (20)
#define TRUE (1)
#define FALSE (0)

/* Public variables and functions */
FILE *ASM_open(char *fname);    /* Open a SIC/XE asm file */
void ASM_close(void);           /* Close the asm file */
int ASM_token(char *buf);       /* Get a token from the file */

/* Private variable and functions */
FILE *ASM_fp;
int ASM_buf;
int ASM_flag = FALSE;
char DELIMITER[] = " ,\t\r\n";
int LEN_DELIMITER = sizeof(DELIMITER)-1;
char SPECIAL[] = "#@+*,.";
int LEN_SPECIAL = sizeof(SPECIAL)-1;

FILE *ASM_open(char *fname) {
    ASM_fp = fopen(fname, "r");
    return ASM_fp;
}

void ASM_close(void) {
    fclose(ASM_fp);
}

int ASM_getc(void) {
    if (ASM_flag) {
        ASM_flag = FALSE;
        return ASM_buf;
    }
    return fgetc(ASM_fp);
}

void ASM_ungetc(int c) {
    ASM_flag = TRUE;
    ASM_buf = c;
}

int is_delimiter(int c) {
    for (int i = 0; i < LEN_DELIMITER; i++)
        if (c == DELIMITER[i])
            return TRUE;
    return FALSE;
}

int is_special(int c) {
    for (int i = 0; i < LEN_SPECIAL; i++)
        if (c == SPECIAL[i])
            return TRUE;
    return FALSE;
}

int ASM_token(char *buf) {
    int c, len;
    buf[0] = '\0';

    c = ASM_getc();
    while (c == ' ' || c == '\t') c = ASM_getc();
    if (c == EOF) return EOF;

    if (is_special(c)) {
        buf[0] = c;
        buf[1] = '\0';
        len = 1;
    } else if (c == '\r' || c == '\n') {
        c = ASM_getc();
        if (c != '\r' && c != '\n') ASM_ungetc(c);
        buf[0] = '\n';
        buf[1] = '\0';
        len = 1;
    } else {
        for (len = 0; !is_delimiter(c) && c != EOF; c = ASM_getc()) {
            if (len < LEN_SYMBOL - 1) {
                buf[len++] = c;
            }
        }
        buf[len] = '\0';
        if (c != ' ' && c != '\t') ASM_ungetc(c);
    }
    return len;
}
