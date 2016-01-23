#include "common.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#include "fail.h"


#define S_Z 1
#define S_C 2


unsigned char pmem[] = {
    0xD0 | 0x02, // mov x, 0x2
    0x50 | 0x01, // mov r1, x
    0xD0 | 0x00, // mov x, 0x0
    0x50 | 0x00, // mov r0, x
    0xD0 | 0x0A, // mov x, 0xA
    0x28 | 0x00, // mov (2r0), x
    0xE0 | 0x1F, // b -1
};


struct processor {
    size_t pc;
    unsigned char regs[16];
    unsigned char x;
    unsigned char s;
    unsigned char* dmem;
    unsigned char* pmem;
};


void store(struct processor* p, size_t addr)
{
    if (addr == 0x20) {
        printf("DEBUG 0x%X\n", p->x);
    } else {
        p->dmem[addr] = p->x;
    }
}


unsigned char load(struct processor* p, size_t addr)
{
    return p->dmem[addr];
}


void exec_insn(struct processor* p)
{
    unsigned int insn = p->pmem[p->pc++];

    if (insn == 0x3F) {
        fatal(E_RARE, "Unimplemented");
    } else if ((insn & 0xFC) == 0x00) {
        // ifc b
        if ((1 << (insn & 0x03)) & p->s)
            ++p->pc;
    } else if ((insn & 0xFC) == 0x04) {
        // ifs b
        if ( !((1 << (insn & 0x03)) & p->s) )
            ++p->pc;
    } else if (insn == 0x0E) {
        // com
        p->x = (~p->x) & 0xF;
    } else if (insn == 0x0F) {
        // asr
        bool neg = p->x & 0x8;
        p->x >>= 1;
        if (neg)
            p->x |= 0x8;
    } else if (insn == 0x10) {
        // sl
        p->x <<= 1;
    } else if (insn == 0x11) {
        // sr
        p->x >>= 1;
    } else if (insn == 0x12) {
        // rl
        p->x = (p->x << 1) | (p->s & S_C ? 0x1 : 0x0);
    } else if (insn == 0x13) {
        // rr
        p->x = (p->x >> 1) | (p->s & S_C ? 0x8 : 0x0);
    } else if ((insn & 0xF0) == 0x20) {
        // mov x, (2r)
        // mov (2r), x
        unsigned char* r = p->regs + (insn & 0x07);
        size_t a = (r[1] << 4) + r[0];
        if (insn & 0x08)
            store(p, a);
        else
            p->x = load(p, a);
    } else if ((insn & 0xF0) == 0x40) {
        // mov x, r
        p->x = p->regs[insn & 0x07];
    } else if ((insn & 0xF0) == 0x50) {
        // mov r, x
        p->regs[insn & 0x07] = p->x;
    } else if ((insn & 0xF0) == 0xD0) {
        // mov x, k
        p->x = insn & 0x0F;
    } else if ((insn & 0xE0) == 0xE0) {
        // b k
        unsigned int k = insn & 0x1F;
        if (k < 0x10)
            p->pc += k;
        else
            p->pc -= (k ^ 0x1F) + 1;
    } else {
        fatal(E_RARE, "Unimplemented");
    }
}


int main()
{
    unsigned char* dmem = mmap(
        NULL, 1<<24, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0);
    if (dmem == MAP_FAILED)
        fatal_e(E_RARE, "Can't mmap data memory");

    struct processor p = {
        .pc = 0,
        .dmem = dmem,
        .pmem = pmem,
    };

    while (true) {
        exec_insn(&p);
    }

    return 0;
}
