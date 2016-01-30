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
    0x21 | 0x00, // mov (2r0), x
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


void store(const struct processor* const p, const size_t addr)
{
    if (addr == 0x20) {
        printf("DEBUG 0x%X\n", p->x);
    } else {
        p->dmem[addr] = p->x;
    }
}


unsigned char load(const struct processor* const p, const size_t addr)
{
    return p->dmem[addr];
}


void set_z(struct processor* const p, unsigned int val)
{
    p->s = (p->s & ~S_Z) | ((val & 0xFF) == 0 ? S_Z : 0);
}


void set_zc(struct processor* const p, unsigned int val)
{
    p->s = (p->s & ~(S_Z | S_C)) |
        ((val & 0xFF) == 0 ? S_Z : 0) | ((val & 0x80) ? S_C : 0);
}


void set_c(struct processor* const p, unsigned int val)
{
    p->s = (p->s & ~S_C) | ((val & 0x80) ? S_C : 0);
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
        set_z(p, p->x);
    } else if (insn == 0x0F) {
        // asr
        bool neg = p->x & 0x8;
        p->x >>= 1;
        if (neg)
            p->x |= 0x8;
        set_z(p, p->x);
    } else if (insn == 0x10) {
        // sl
        p->x <<= 1;
        set_z(p, p->x);
    } else if (insn == 0x11) {
        // sr
        p->x >>= 1;
        set_z(p, p->x);
    } else if (insn == 0x12) {
        // rl
        p->x = (p->x << 1) | (p->s & S_C ? 0x1 : 0x0);
        set_z(p, p->x);
    } else if (insn == 0x13) {
        // rr
        p->x = (p->x >> 1) | (p->s & S_C ? 0x8 : 0x0);
        set_z(p, p->x);
    } else if ((insn & 0xF0) == 0x20) {
        // mov x, (2r)
        // mov (2r), x
        unsigned char* r = p->regs + (insn & 0x0E);
        size_t a = (r[1] << 4) | r[0];
        if (insn & 0x01) {
            store(p, a);
        } else {
            p->x = load(p, a);
            set_z(p, p->x);
        }
    } else if ((insn & 0xF2) == 0x30) {
        // mov x, (4r)
        // mov (4r), x
        unsigned char* r = p->regs + (insn & 0x0C);
        size_t a = (r[3] << 12) | (r[2] << 8) | (r[1] << 4) | r[0];
        if (insn & 0x01) {
            store(p, a);
        } else {
            p->x = load(p, a);
            set_z(p, p->x);
        }
    } else if ((insn & 0xF6) == 0x32) {
        // mov x, (8r)
        // mov (8r), x
        unsigned char* r = p->regs + (insn & 0x08);
        size_t a = (r[7] << 28) | (r[6] << 24) | (r[5] << 20) | (r[4] << 16) |
            (r[3] << 12) | (r[2] << 8) | (r[1] << 4) | r[0];
        if (insn & 0x01) {
            store(p, a);
        } else {
            p->x = load(p, a);
            set_z(p, p->x);
        }
    } else if ((insn & 0xF7) == 0x36) {
        // edec (8r)
        unsigned char* r = p->regs + (insn & 0x08);
        size_t a = (r[7] << 28) | (r[6] << 24) | (r[5] << 20) | (r[4] << 16) |
            (r[3] << 12) | (r[2] << 8) | (r[1] << 4) | r[0];
        if (p->x == load(p, a)) {
            --p->x;
            store(p, a);
        }
    } else if (insn == 0x37) {
        // proc
        p->x = 0;
    } else if (insn == 0x3F) {
        // EXTENDED INSTRUCTION
        fatal(E_COMMON, "Unimplemented");
    } else if ((insn & 0xF0) == 0x40) {
        // mov x, r
        p->x = p->regs[insn & 0x0F];
    } else if ((insn & 0xF0) == 0x50) {
        // mov r, x
        p->regs[insn & 0x0F] = p->x;
    } else if ((insn & 0xF0) == 0x60) {
        // and r
        set_z(p, p->regs[insn & 0x0F] &= p->x);
    } else if ((insn & 0xF0) == 0x70) {
        // or r
        set_z(p, p->regs[insn & 0x0F] |= p->x);
    } else if ((insn & 0xE0) == 0x80) {
        // add r
        // adc r
        unsigned char* r = p->regs + (insn & 0x0F);
        unsigned int res = *r + p->x;
        if ((insn & 0x10) && (p->s & S_C))
            ++res;
        *r = res & 0xFF;

        if (!(p->s & S_Z) && (insn & 0x10))
            set_c(p, res);
        else
            set_zc(p, res);
    } else if ((insn & 0xE0) == 0xA0) {
        // sub r
        // sbc r
        unsigned char* r = p->regs + (insn & 0x0F);
        unsigned int res = *r + ((~p->x + 1) & 0xFF);
        if ((insn & 0x10) && !(p->s & S_C))
            ++res;
        *r = res & 0xFF;
        p->s = (p->s & ~S_C) | (res & 0x100 ? S_C : 0);
        if (!(p->s & S_Z) && (insn & 0x10))
            set_c(p, res);
        else
            set_zc(p, res);
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
