#ifndef OPTABLE_H
#define OPTABLE_H

#define FMT0 0x00
#define FMT1 0x01
#define FMT2 0x02
#define FMT3 0x04
#define FMT4 0x08

#define OP_BYTE 0x101
#define OP_WORD 0x102
#define OP_RESB 0x103
#define OP_RESW 0x104
#define OP_BASE 0x105
#define OP_NOBASE 0x106
#define OP_START 0x107
#define OP_END 0x108

typedef struct {
    char op[20];
    unsigned fmt;
    unsigned code;
} Instruction;

Instruction *is_opcode(char *op);

#endif
