/*INPUT IN CMD
gcc -o main main.c
main test.asm
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

typedef struct
{
    int address;
    int length;
} relocation;

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
    relocation relocation[200];
    int relocation_count = 0;

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
                    printf("  !!!!!!!!   Error   !!!!!!!!\n", place);
                else if (c == LINE_COMMENT)
                    printf("   ========   Comment_line   ========\n");
                else
                {
                    printf("%06X : %9s %9s %9s,%9s \n", line[line_count].place, line[line_count].symbol, line[line_count].op, line[line_count].operand1, line[line_count].operand2, place);
                }
            }
            ASM_close();
        }
        printf("\n--------------------------------------------------------");
        printf("\n\nProgram length: %X\n", place - line[1].place);

        int k = 1;
        while (k <= line_count)
        {
            if (line[k].symbol[0] != '\0' && line[k].code != 0x107)
            {
                printf("%7s : %06X\n", line[k].symbol, line[k].place);
            }
            k++;
        }

        printf("\n=========================================================\n\n");

        if (line[1].code == 0x107)
            printf("H%-06s%06X%06X\n", line[1].symbol, line[1].place, place - line[1].place);
        else
            printf("H%-06s%06X%06X\n", "Null", 0x0, place);

        int base = 0;
        int enable_base = 0;
        int start_address = 0;
        int start_position = 0;

        if (line[line_count - 1].fmt == FMT1)
        {
            line[line_count].place = line[line_count - 1].place + 1;
        }
        if (line[line_count - 1].fmt == FMT2)
        {
            line[line_count].place = line[line_count - 1].place + 2;
        }
        if (line[line_count - 1].fmt == FMT3)
        {
            line[line_count].place = line[line_count - 1].place + 3;
        }
        if (line[line_count - 1].fmt == FMT4)
        {
            line[line_count].place = line[line_count - 1].place + 4;
        }
        if (line[line_count - 1].code == OP_BYTE)
        {
            line[line_count].place = line[line_count - 1].place + 1;
        }
        if (line[line_count - 1].code == OP_WORD)
        {
            line[line_count].place = line[line_count - 1].place + 3;
        }

        int j;
        for (j = 0; j < line_count && line[j + 1].code != OP_RESB && line[j + 1].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
            ;
        // printf("\n j: %d\n", j);
        start_position += start_address;
        start_address = 0;
        // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
        printf("T%06X%02X", start_position, line[j].place - start_position);

        for (int i = 0; i < line_count; i++)
        {
            // printf("\nstart_address: %d, start position: %d\n", start_address, start_position);
            // printf("line %d: FMT %d\n", i, line[i].fmt);
            if (line[i].fmt == FMT1)
            {
                if (start_position == -1)
                {
                    start_position = line[i].place;
                    start_address = 0;
                    for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                        ;
                    // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                    printf("\nT%06X%02X", start_position, line[j].place - start_position);
                }
                if (start_address + 1 > 30)
                {
                    int j;
                    start_position += start_address;
                    start_address = 0;
                    for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                        ;
                    // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                    printf("\nT%06X%02X", start_position, line[j].place - start_position);
                }
                printf("%02X", line[i].code);
                start_address += 1;
            }
            else if (line[i].fmt == FMT2)
            {
                int value1, value2;
                switch (line[i].operand1[0])
                {
                case 'A':
                    value1 = 0;
                    break;
                case 'X':
                    value1 = 1;
                    break;
                case 'L':
                    value1 = 2;
                    break;
                case 'B':
                    value1 = 3;
                    break;
                case 'S':
                    value1 = 4;
                    break;
                case 'T':
                    value1 = 5;
                    break;
                case 'F':
                    value1 = 6;
                    break;
                default:
                    value1 = -1;
                    break;
                }

                if (strlen(line[i].operand2) > 0)
                {
                    switch (line[i].operand2[0])
                    {
                    case 'A':
                        value2 = 0;
                        break;
                    case 'X':
                        value2 = 1;
                        break;
                    case 'L':
                        value2 = 2;
                        break;
                    case 'B':
                        value2 = 3;
                        break;
                    case 'S':
                        value2 = 4;
                        break;
                    case 'T':
                        value2 = 5;
                        break;
                    case 'F':
                        value2 = 6;
                        break;
                    default:
                        value2 = -1;
                        break;
                    }

                    if (start_position == -1)
                    {
                        start_position = line[i].place;
                        start_address = 0;
                        for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                            ;
                        // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                        printf("\nT%06X%02X", start_position, line[j].place - start_position);
                    }
                    if (start_address + 2 > 30)
                    {
                        int j;
                        start_position += start_address;
                        start_address = 0;
                        for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                            ;
                        printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                    }
                    printf("%02X%d%d", line[i].code, value1, value2);
                    start_address += 2;
                }
                else
                {
                    if (start_position == -1)
                    {
                        start_position = line[i].place;
                        start_address = 0;
                        for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                            ;
                        // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                        printf("\nT%06X%02X", start_position, line[j].place - start_position);
                    }
                    if (start_address + 2 > 30)
                    {
                        int j;
                        start_position += start_address;
                        start_address = 0;
                        for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                            ;
                        // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                        printf("\nT%06X%02X", start_position, line[j].place - start_position);
                    }
                    printf("%02X%d0", line[i].code, value1);
                    start_address += 2;
                }
            }

            else if (line[i].fmt == FMT3)
            {
                if (line[i].code == 0x4C)
                {
                    if (start_position == -1)
                    {
                        start_position = line[i].place;
                        start_address = 0;
                        for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                            ;
                        // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                        printf("\nT%06X%02X", start_position, line[j].place - start_position);
                    }
                    if (start_address + 3 > 30)
                    {
                        int j;
                        start_position = line[i].place;
                        start_address = 0;
                        for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                            ;
                        // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                        printf("\nT%06X%02X", start_position, line[j].place - start_position);
                    }
                    printf("4F0000");
                    start_address += 3;
                }
                else
                {
                    int operand_value = 0;
                    int loc = line[i].place;
                    int pc = loc + 3;

                    int found = 0;
                    for (int j = 1; j <= line_count; j++)
                    {
                        if (strcmp(line[j].symbol, line[i].operand1) == 0)
                        {
                            found = 1;
                            operand_value = line[j].place;
                            break;
                        }
                        else if ((line[i].operand1[0] == '#' || line[i].operand1[0] == '@') && strcmp(line[j].symbol, line[i].operand1 + 1) == 0)
                        {
                            found = 1;
                            operand_value = line[j].place;
                            break;
                        }
                    }
                    int pc_relative = 0;
                    int base_relative = 0;
                    int nothing = 0;
                    int sic_standard = 0;
                    int disp = 0;

                    if (!found)
                    {
                        operand_value = atoi(line[i].operand1 + 1);
                        disp = operand_value;
                    }

                    else
                    {
                        if (operand_value - pc >= -2048 && operand_value - pc <= 2047) // PC Relative
                        {
                            pc_relative = 1;
                            disp = operand_value - pc;
                        }

                        else if (enable_base && operand_value - base >= 0 && operand_value - base <= 4095) // Base Relative
                        {
                            base_relative = 1;
                            disp = operand_value - base;
                        }

                        else if (operand_value < 4096) // Not PC or Base
                        {
                            nothing = 1;
                            disp = operand_value;

                            relocation[relocation_count].address = line[i].place + 1;
                            relocation[relocation_count].length = 2;
                            relocation_count++;
                        }

                        else if ((line[i].addressing & ADDR_SIMPLE) != 0 && operand_value < 32767) // SIC Standard
                        {
                            sic_standard = 1;
                            disp = operand_value;

                            relocation[relocation_count].address = line[i].place + 1;
                            relocation[relocation_count].length = 2;
                            relocation_count++;
                        }
                        else // Error
                        {
                        }
                    }

                    int xbpe = 0;
                    int op3 = 0;
                    if ((line[i].addressing & ADDR_IMMEDIATE) != 0)
                    {
                        op3 = 1;
                    }
                    else if ((line[i].addressing & ADDR_INDIRECT) != 0)
                    {
                        op3 = 2;
                    }
                    else if ((line[i].addressing & ADDR_SIMPLE) != 0)
                    {
                        op3 = 3;
                    }
                    else if (sic_standard)
                    {
                        op3 = 0;
                    }
                    if (pc_relative == 1)
                    {
                        xbpe = 2;
                    }
                    else if (base_relative == 1)
                    {
                        xbpe = 4;
                    }
                    else
                    {
                        xbpe = 0;
                    }
                    if ((line[i].addressing & ADDR_INDEX) != 0)
                    {
                        xbpe += 8;
                    }

                    if (start_position == -1)
                    {
                        start_position = line[i].place;
                        start_address = 0;
                        for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                            ;
                        // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                        printf("\nT%06X%02X", start_position, line[j].place - start_position);
                    }
                    if (start_address + 3 > 30)
                    {
                        int j;
                        start_position += start_address;
                        start_address = 0;
                        for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                            ;
                        // printf("\n j = %d\n", j);
                        // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                        printf("\nT%06X%02X", start_position, line[j].place - start_position);
                    }
                    printf("%02X%X%03X", op3 + line[i].code, xbpe, disp & 0XFFF);
                    start_address += 3;
                }
            }

            else if (line[i].fmt == FMT4)
            {
                int found = 0;
                int symbol_value = 0;
                for (int j = 1; j < line_count; j++)
                {
                    if (strcmp(line[j].symbol, line[i].operand1) == 0)
                    {
                        found = 1;
                        symbol_value = line[j].place;
                        break;
                    }
                }

                if (!found)
                {
                    symbol_value = atoi(line[i].operand1 + 1);
                }
                else
                {
                    relocation[relocation_count].address = line[i].place + 1;
                    relocation[relocation_count].length = 5;
                    relocation_count++;
                }

                int xbpe = 0;
                int op4 = 0;
                if ((line[i].addressing & ADDR_IMMEDIATE) != 0)
                {
                    op4 = 1;
                    xbpe = 1;
                }
                else if ((line[i].addressing & ADDR_INDIRECT) != 0)

                {
                    op4 = 2;
                    xbpe = 1;
                }
                else if ((line[i].addressing & ADDR_SIMPLE) != 0)
                {
                    op4 = 3;
                    xbpe = 1;
                }
                if (line[i].addressing == ADDR_INDEX)
                {
                    xbpe = 9;
                }

                if (start_position == -1)
                {
                    start_position = line[i].place;
                    start_address = 0;
                    for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                        ;
                    // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                    printf("\nT%06X%02X", start_position, line[j].place - start_position);
                }
                if (start_address + 4 > 30)
                {
                    int j;
                    start_position += start_address;
                    start_address = 0;
                    for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                        ;
                    // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                    printf("\nT%06X%02X", start_position, line[j].place - start_position);
                }
                printf("%03X%05X", (op4 + line[i].code) * 16 + xbpe, symbol_value);
                start_address += 4;
            }

            else if (line[i].code == OP_RESB)
            {
                start_position = -1;
            }
            else if (line[i].code == OP_RESW)
            {
                start_position = -1;
            }

            else if (line[i].code == OP_BYTE)
            {
                if (line[i].operand1[0] == 'C' || line[i].operand1[0] == 'c')
                {
                    int length = strlen(line[i].operand1);
                    for (int k = 2; k < length - 1; k++)
                    {
                        if (start_position == -1)
                        {
                            start_position = line[i].place;
                            start_address = 0;
                            for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                                ;
                            // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                            printf("\nT%06X%02X", start_position, line[j].place - start_position);
                        }
                        if (start_address + 1 > 30)
                        {
                            int j;
                            start_position += start_address;
                            start_address = 0;
                            for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                                ;
                            // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                            printf("\nT%06X%02X", start_position, line[j].place - start_position);
                        }
                        printf("%02X", line[i].operand1[k]);
                        start_address += 1;
                    }
                }
                else if (line[i].operand1[0] == 'X' || line[i].operand1[0] == 'x')
                {
                    int length = strlen(line[i].operand1);
                    for (int k = 2; k < length - 1; k += 2)
                    {
                        if (start_position == -1)
                        {
                            start_position = line[i].place;
                            start_address = 0;
                            for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                                ;
                            // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                            printf("\nT%06X%02X", start_position, line[j].place - start_position);
                        }
                        if (start_address + 1 > 30)
                        {
                            int j;
                            start_position += start_address;
                            start_address = 0;
                            for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                                ;
                            // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                            printf("\nT%06X%02X", start_position, line[j].place - start_position);
                        }
                        printf("%c%c", line[i].operand1[k], line[i].operand1[k + 1]);
                        start_address += 1;
                    }
                }
            }

            else if (line[i].code == OP_WORD)
            {
                int word_value = atoi(line[i].operand1);
                if (start_position == -1)
                {
                    start_position = line[i].place;
                    start_address = 0;
                    for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                        ;
                    // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                    printf("\nT%06X%02X", start_position, line[j].place - start_position);
                }
                if (start_address + 3 > 30)
                {
                    int j;
                    start_position += start_address;
                    start_address = 0;
                    for (j = i; j < line_count - 1 && line[j].code != OP_RESB && line[j].code != OP_RESW && line[j + 1].place - start_position <= 30; j++)
                        ;
                    // printf("%d %d %d %d %s\n", j, start_address, start_position, line[j].place, line[j].op);
                    printf("\nT%06X%02X", start_position, line[j].place - start_position);
                }
                printf("%06X", word_value);
                start_address += 3;
            }

            else if (line[i].code == OP_BASE)
            {
                enable_base = 1;
                for (int j = 0; j < line_count; j++)
                {
                    if (strcmp(line[i].operand1, line[j].symbol) == 0)
                    {
                        base = line[j].place;
                        break;
                    }
                }
                // printf("\n base %d \n", base);
            }

            else if (line[i].code == OP_NOBASE)
            {
                enable_base = 0;
            }
        }

        for (int j = 0; j < relocation_count; j++)
        {
            printf("\nM%06X%02X", relocation[j].address, relocation[j].length);
        }

        int end_address = 0;

        if (line[line_count - 1].code == OP_END)
        {
            for (int i = 0; i < line_count; i++)
            {
                if (strcmp(line[line_count - 1].operand1, line[i].symbol) == 0)
                {
                    end_address = line[i].place;
                    break;
                }
            }
        }

        if (end_address != 0)
        {
            printf("\nE%06X\n", end_address);
        }
        else
        {
            printf("\nE000000\n");
        }
    }

    return 0;
}
