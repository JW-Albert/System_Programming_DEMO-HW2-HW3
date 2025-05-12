/*
open CMD and input:
gcc -o main main.c
main sample.asm
*/
#include <string.h>
#include <stdlib.h>
#include "2-optable.c"

/* Public variables and functions */
#define ADDR_SIMPLE 0x01
#define ADDR_IMMEDIATE 0x02
#define ADDR_INDIRECT 0x04
#define ADDR_INDEX 0x08

#define LINE_EOF (-1)
#define LINE_COMMENT (-2)
#define LINE_ERROR (0)
#define LINE_CORRECT (1)

typedef struct
{
    char symbol[LEN_SYMBOL];
    char op[LEN_SYMBOL];
    char operand1[LEN_SYMBOL];
    char operand2[LEN_SYMBOL];
    unsigned code;
    unsigned fmt;
    unsigned addressing;
    int place;
} LINE;

int process_line(LINE *line, int *place);
/* return LINE_EOF, LINE_COMMENT, LINE_ERROR, LINE_CORRECT and Instruction information in *line*/

/* Private variable and function */

void init_LINE(LINE *line)
{
    line->symbol[0] = '\0';
    line->op[0] = '\0';
    line->operand1[0] = '\0';
    line->operand2[0] = '\0';
    line->code = 0x0;
    line->fmt = 0x0;
    line->addressing = ADDR_SIMPLE;
}

int process_line(LINE *line, int *place)
/* return LINE_EOF, LINE_COMMENT, LINE_ERROR, LINE_CORRECT */
{
    char buf[LEN_SYMBOL];
    int c;
    int state;
    int ret;
    Instruction *op;

    c = ASM_token(buf); /* get the first token of a line */
    if (c == EOF)
        return LINE_EOF;
    else if ((c == 1) && (buf[0] == '\n')) /* blank line */
        return LINE_COMMENT;
    else if ((c == 1) && (buf[0] == '.')) /* a comment line */
    {
        do
        {
            c = ASM_token(buf);
        } while ((c != EOF) && (buf[0] != '\n'));
        return LINE_COMMENT;
    }
    else
    {
        init_LINE(line);
        ret = LINE_ERROR;
        state = 0;
        while (state < 8)
        {
            switch (state)
            {
            case 0:
            case 1:
            case 2:
                op = is_opcode(buf);
                if ((state < 2) && (buf[0] == '+')) /* + */
                {
                    line->fmt = FMT4;
                    line->op[0] = '+';
                    line->place = *place;
                    *place = *place + 4;
                    state = 2;
                }
                else if (op != NULL) /* INSTRUCTION */
                {
                    strcat(line->op, op->op);
                    line->code = op->code;
                    state = 3;
                    if (line->fmt != FMT4)
                    {
                        line->place = *place;
                        line->fmt = op->fmt & (FMT1 | FMT2 | FMT3);
                        if (line->fmt == FMT1)
                        {
                            *place = *place + 1;
                        }
                        else if (line->fmt == FMT2)
                        {
                            *place = *place + 2;
                        }
                        else if (line->fmt == FMT3)
                        {
                            *place = *place + 3;
                        }
                    }
                    else if ((line->fmt == FMT4) && ((op->fmt & FMT4) == 0)) /* INSTRUCTION is FMT1 or FMT 2*/
                    {                                                        /* ERROR 20210326 added */
                        printf("ERROR at token %s, %s cannot use format 4 \n", buf, buf);
                        ret = LINE_ERROR;
                        state = 7; /* skip following tokens in the line */
                    }
                }
                else if (state == 0) /* SYMBOL */
                {
                    strcat(line->symbol, buf);
                    state = 1;
                }
                else /* ERROR */
                {
                    printf("ERROR at token %s\n", buf);
                    ret = LINE_ERROR;
                    state = 7; /* skip following tokens in the line */
                }
                break;
            case 3:
                if (line->fmt == FMT1 || line->code == 0x4C) /* no operand needed */
                {
                    if (c == EOF || buf[0] == '\n')
                    {
                        ret = LINE_CORRECT;
                        state = 8;
                    }
                    else /* COMMENT */
                    {
                        ret = LINE_CORRECT;
                        state = 7;
                    }
                }
                else
                {
                    if (c == EOF || buf[0] == '\n')
                    {
                        ret = LINE_ERROR;
                        state = 8;
                    }
                    else if (buf[0] == '@' || buf[0] == '#')
                    {
                        line->addressing = (buf[0] == '#') ? ADDR_IMMEDIATE : ADDR_INDIRECT;
                        if (line->addressing == ADDR_INDIRECT)
                        {
                            line->operand1[0] = '@';
                        }
                        else if (line->addressing == ADDR_IMMEDIATE)
                        {
                            line->operand1[0] = '#';
                        }
                        state = 4;
                    }
                    else /* get a symbol */
                    {
                        op = is_opcode(buf);
                        if (op != NULL)
                        {
                            printf("Operand1 cannot be a reserved word\n");
                            ret = LINE_ERROR;
                            state = 7; /* skip following tokens in the line */
                        }
                        else
                        {
                            strcat(line->operand1, buf);
                            state = 5;
                        }
                    }
                }
                break;
            case 4:
                op = is_opcode(buf);
                if (op != NULL)
                {
                    printf("Operand1 cannot be a reserved word\n");
                    ret = LINE_ERROR;
                    state = 7; /* skip following tokens in the line */
                }
                else
                {
                    strcat(line->operand1, buf);
                    state = 5;
                }
                break;
            case 5:
                if (c == EOF || buf[0] == '\n')
                {
                    if (line->fmt == FMT0 && line->code == 0x107)
                    {
                        *place += atoi(line->operand1);
                        line->place = *place;
                    }
                    else if (line->fmt == FMT0 && line->code == 0x101)
                    {
                        if (line->operand1[0] == 'X' || line->operand1[0] == 'x')
                        {
                            line->place = *place;
                            *place = *place + 1;
                        }
                        else if (line->operand1[0] == 'C' || line->operand1[0] == 'c')
                        {
                            line->place = *place;
                            int j = 2;
                            int tmp = 0;
                            while (line->operand1[j] != '\'')
                            {
                                tmp++;
                                j++;
                            }
                            *place += tmp;
                        }
                        else
                        {
                            line->place = *place;
                            *place = *place + 1;
                        }
                    }
                    else if (line->fmt == FMT0 && line->code == 0x102)
                    {
                        line->place = *place;
                        *place = *place + 3;
                    }
                    else if (line->fmt == FMT0 && line->code == 0x103)
                    {
                        line->place = *place;
                        *place = *place + atoi(line->operand1);
                    }
                    else if (line->fmt == FMT0 && line->code == 0x104)
                    {
                        line->place = *place;
                        *place = *place + atoi(line->operand1) * 3;
                    }
                    ret = LINE_CORRECT;
                    state = 8;
                }
                else if (buf[0] == ',')
                {
                    state = 6;
                }
                else /* COMMENT */
                {
                    ret = LINE_CORRECT;
                    state = 7; /* skip following tokens in the line */
                }
                break;
            case 6:
                if (c == EOF || buf[0] == '\n')
                {
                    ret = LINE_ERROR;
                    state = 8;
                }
                else /* get a symbol */
                {
                    op = is_opcode(buf);
                    if (op != NULL)
                    {
                        printf("Operand2 cannot be a reserved word\n");
                        ret = LINE_ERROR;
                        state = 7; /* skip following tokens in the line */
                    }
                    else
                    {
                        if (line->fmt == FMT2)
                        {
                            strcat(line->operand2, buf);
                            ret = LINE_CORRECT;
                            state = 7;
                        }
                        else if ((c == 1) && (buf[0] == 'x' || buf[0] == 'X'))
                        {
                            line->addressing = line->addressing | ADDR_INDEX;
                            strcat(line->operand2, buf);
                            ret = LINE_CORRECT;
                            state = 7; /* skip following tokens in the line */
                        }
                        else
                        {
                            printf("Operand2 exists only if format 2  is used\n");
                            ret = LINE_ERROR;
                            state = 7; /* skip following tokens in the line */
                        }
                    }
                }
                break;
            case 7: /* skip tokens until '\n' || EOF */
                if (c == EOF || buf[0] == '\n')
                    state = 8;
                break;
            }
            if (state < 8)
                c = ASM_token(buf); /* get the next token */
        }
        return ret;
    }
}

int main(int argc, char *argv[])
{
    int i, c, line_count;
    char buf[LEN_SYMBOL];
    LINE line[500];
    int place = 0;

    if (argc < 2)
    {
        printf("Usage: %s fname.asm\n", argv[0]);
    }
    else
    {
        if (ASM_open(argv[1]) == NULL)
            printf("File not found!!\n");
        else
        {
            for (line_count = 1; (c = process_line(&line[line_count], &place)) != LINE_EOF; line_count++)
            {

                if (c == LINE_ERROR)
                    printf("!!!!!!!!   Error   !!!!!!!!\n", place);
                else if (c == LINE_COMMENT)
                    printf("========   Comment_line   ========\n");
                else
                {
                    printf("%06X : %9s %9s %9s,%9s \n", line[line_count].place, line[line_count].symbol, line[line_count].op, line[line_count].operand1, line[line_count].operand2, place);
                }
            }
            ASM_close();
        }
        printf("\n--------------------------------------------------------");
        printf("\n\nProgram length: %X\n", place);

        int k = 1;
        while (k <= line_count)
        {
            if (line[k].symbol[0] != '\0' && line[k].code != 0x107)
            {
                printf("%7s : %06X\n", line[k].symbol, line[k].place);
            }
            k++;
        }
    }

    return 0;
}
