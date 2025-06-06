#include <string.h>
#include "1-token.h"
#include "2-optable.h"

Instruction OPTAB[] = {
    {"ADD", FMT3 | FMT4, 0x18}, {"ADDF", FMT3 | FMT4, 0x58}, {"ADDR", FMT2, 0x90},
    {"AND", FMT3 | FMT4, 0x40}, {"BASE", FMT0, OP_BASE}, {"BYTE", FMT0, OP_BYTE},
    {"CLEAR", FMT2, 0xB4}, {"COMP", FMT3 | FMT4, 0x28}, {"COMPF", FMT3 | FMT4, 0x88},
    {"COMPR", FMT2, 0xA0}, {"DIV", FMT3 | FMT4, 0x24}, {"DIVF", FMT3 | FMT4, 0x64},
    {"DIVR", FMT2, 0x9C}, {"END", FMT0, OP_END}, {"FIX", FMT1, 0xC4},
    {"FLOAT", FMT1, 0xC0}, {"HIO", FMT1, 0xF4}, {"J", FMT3 | FMT4, 0x3C},
    {"JEQ", FMT3 | FMT4, 0x30}, {"JGT", FMT3 | FMT4, 0x34}, {"JLT", FMT3 | FMT4, 0x38},
    {"JSUB", FMT3 | FMT4, 0x48}, {"LDA", FMT3 | FMT4, 0x00}, {"LDB", FMT3 | FMT4, 0x68},
    {"LDCH", FMT3 | FMT4, 0x50}, {"LDF", FMT3 | FMT4, 0x70}, {"LDL", FMT3 | FMT4, 0x08},
    {"LDS", FMT3 | FMT4, 0x6C}, {"LDT", FMT3 | FMT4, 0x74}, {"LDX", FMT3 | FMT4, 0x04},
    {"LPS", FMT3 | FMT4, 0xD0}, {"MUL", FMT3 | FMT4, 0x20}, {"MULF", FMT3 | FMT4, 0x60},
    {"MULR", FMT2, 0x98}, {"NOBASE", FMT0, OP_NOBASE}, {"NORM", FMT1, 0xC8},
    {"OR", FMT3 | FMT4, 0x44}, {"RD", FMT3 | FMT4, 0xD8}, {"RESB", FMT0, OP_RESB},
    {"RESW", FMT0, OP_RESW}, {"RMO", FMT2, 0xAC}, {"RSUB", FMT3 | FMT4, 0x4C},
    {"SHIFTL", FMT2, 0xA4}, {"SHIFTR", FMT2, 0xA8}, {"SIO", FMT1, 0xF0},
    {"SSK", FMT3 | FMT4, 0xEC}, {"STA", FMT3 | FMT4, 0x0C}, {"START", FMT0, OP_START},
    {"STB", FMT3 | FMT4, 0x78}, {"STCH", FMT3 | FMT4, 0x54}, {"STF", FMT3 | FMT4, 0x80},
    {"STI", FMT3 | FMT4, 0xD4}, {"STL", FMT3 | FMT4, 0x14}, {"STS", FMT3 | FMT4, 0x7C},
    {"STSW", FMT3 | FMT4, 0xE8}, {"STT", FMT3 | FMT4, 0x84}, {"STX", FMT3 | FMT4, 0x10},
    {"SUB", FMT3 | FMT4, 0x1C}, {"SUBF", FMT3 | FMT4, 0x5C}, {"SUBR", FMT2, 0x94},
    {"SVC", FMT2, 0xB0}, {"TD", FMT3 | FMT4, 0xE0}, {"TIO", FMT1, 0xF8},
    {"TIX", FMT3 | FMT4, 0x2C}, {"TIXR", FMT2, 0xB8}, {"WD", FMT3 | FMT4, 0xDC},
    {"WORD", FMT0, OP_WORD}
};
int LEN_OPTAB = sizeof(OPTAB) / sizeof(Instruction);

Instruction *is_opcode(char *op) {
    int begin = 0, end = LEN_OPTAB - 1, mid, c;
    char buf[LEN_SYMBOL], *p;
    for (c = 0, p = op; *p != '\0'; c++, p++)
        buf[c] = (*p >= 'a' && *p <= 'z') ? *p - 'a' + 'A' : *p;
    buf[c] = '\0';

    while (begin <= end) {
        mid = (begin + end) / 2;
        c = strcmp(buf, OPTAB[mid].op);
        if (c == 0)
            return &OPTAB[mid];
        else if (c < 0)
            end = mid - 1;
        else
            begin = mid + 1;
    }
    return NULL;
}
