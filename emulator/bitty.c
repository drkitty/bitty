#include "common.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#include "fail.h"
#include "insn.h"


#define S_Z 1
#define S_C 2


struct processor {
    size_t pc;
    uint8_t regs[16];
    uint8_t x;
    uint8_t s;
    uint8_t* dmem;
    uint8_t* pmem;
};


void store(const struct processor* const p, const size_t addr)
{
    if (addr == 0x20) {
        printf("DEBUG 0x%X\n", p->x);
    } else {
        p->dmem[addr] = p->x;
    }
}


uint8_t load(const struct processor* const p, const size_t addr)
{
    return p->dmem[addr];
}


void set_z(struct processor* const p, uint8_t val)
{
    p->s = (p->s & ~S_Z) | ((val & 0xF) == 0 ? S_Z : 0);
}


void set_zc(struct processor* const p, uint8_t val)
{
    p->s = (p->s & ~(S_Z | S_C))
        | ((val & 0xF) == 0 ? S_Z : 0) | ((val & 0x10) ? S_C : 0);
}


void set_c(struct processor* const p, uint8_t val)
{
    p->s = (p->s & ~S_C) | ((val & 0x10) ? S_C : 0);
}


uint8_t get_2r(const struct processor* const p, unsigned int n)
{
    const uint8_t* r = p->regs + n;
    return r[0] | (r[1] << 4);
}


uint16_t get_4r(const struct processor* const p, unsigned int n)
{
    const uint8_t* r = p->regs + n;
    return r[0] | (r[1] << 4) | (r[2] << 8) | (r[3] << 12);
}


uint32_t get_8r(const struct processor* const p, unsigned int n)
{
    const uint8_t* r = p->regs + n;
    return (
        r[0] | (r[1] << 4) | (r[2] << 8) | (r[3] << 12) | (r[4] << 16)
        | (r[5] << 20) | (r[6] << 24) | (r[7] << 28)
    );
}


void exec_insn(struct processor* p)
{
    uint8_t insn = p->pmem[p->pc++];

    if ((insn & I_BR_2R.m) == I_BR_2R.c) {
        // br 2r
        p->pc += get_2r(p, insn & ~I_BR_2R.m);
    } else if ((insn & I_BR_4R.m) == I_BR_4R.c) {
        // br 4r
        p->pc += get_4r(p, insn & ~I_BR_4R.m);
    } else if ((insn & I_BR_8R.m) == I_BR_8R.c) {
        // br 8r
        p->pc += get_8r(p, insn & ~I_BR_8R.m);
    } else if ((insn & I_IFC_B.m) == I_IFC_B.c) {
        // ifc b
        if ((1 << (insn & 0x03)) & p->s)
            ++p->pc;
    } else if ((insn & I_IFS_B.m) == I_IFS_B.c) {
        // ifs b
        if ( !((1 << (insn & ~I_IFS_B.m)) & p->s) )
            ++p->pc;
    } else if ((insn & I_COM.m) == I_COM.c) {
        // com
        p->x = (~p->x) & 0xF;
        set_z(p, p->x);
    } else if ((insn & I_ASR.m) == I_ASR.c) {
        // asr
        bool neg = p->x & 0x8;
        p->x >>= 1;
        if (neg)
            p->x |= 0x8;
        set_z(p, p->x);
    } else if ((insn & I_SL.m) == I_SL.c) {
        // sl
        p->x <<= 1;
        set_z(p, p->x);
    } else if ((insn & I_SR.m) == I_SR.c) {
        // sr
        p->x >>= 1;
        set_z(p, p->x);
    } else if ((insn & I_RL.m) == I_RL.c) {
        // rl
        p->x = (p->x << 1) | (p->s & S_C ? 0x1 : 0x0);
        set_z(p, p->x);
    } else if ((insn & I_RR.m) == I_RR.c) {
        // rr
        p->x = (p->x >> 1) | (p->s & S_C ? 0x8 : 0x0);
        set_z(p, p->x);
    } else if ((insn & I_LDM_2R.m) == I_LDM_2R.c) {
        // mov x, (2r)
        size_t a = get_2r(p, insn & ~I_LDM_2R.m);
        p->x = load(p, a);
        set_z(p, p->x);
    } else if ((insn & I_STM_2R.m) == I_STM_2R.c) {
        // mov (2r), x
        size_t a = get_2r(p, insn & ~I_STM_2R.m);
        store(p, a);
    } else if ((insn & I_LDM_4R.m) == I_LDM_4R.c) {
        // mov x, (4r)
        size_t a = get_4r(p, insn & ~I_LDM_4R.m);
        p->x = load(p, a);
        set_z(p, p->x);
    } else if ((insn & I_STM_4R.m) == I_STM_4R.c) {
        // mov (4r), x
        size_t a = get_4r(p, insn & ~I_STM_4R.m);
        store(p, a);
    } else if ((insn & I_LDM_8R.m) == I_LDM_8R.c) {
        // mov x, (8r)
        size_t a = get_8r(p, insn & ~I_LDM_8R.m);
        p->x = load(p, a);
        set_z(p, p->x);
    } else if ((insn & I_STM_8R.m) == I_STM_8R.c) {
        // mov (8r), x
        size_t a = get_8r(p, insn & ~I_STM_8R.m);
        store(p, a);
    } else if ((insn & I_EDEC_8R.m) == I_EDEC_8R.c) {
        // edec (8r)
        size_t a = get_8r(p, insn & ~I_EDEC_8R.m);
        if (p->x == load(p, a)) {
            --p->x;
            store(p, a);
        }
    } else if ((insn & I_PROC.m) == I_PROC.c) {
        // proc
        p->x = 0;
    } else if ((insn & I_LDR.m) == I_LDR.c) {
        // mov x, r
        p->x = p->regs[insn & ~I_LDR.m];
    } else if ((insn & I_STR.m) == I_STR.c) {
        // mov r, x
        p->regs[insn & ~I_STR.m] = p->x;
    } else if ((insn & I_AND_R.m) == I_AND_R.c) {
        // and r
        set_z(p, p->regs[insn & ~I_AND_R.m] &= p->x);
    } else if ((insn & I_OR_R.m) == I_OR_R.c) {
        // or r
        set_z(p, p->regs[insn & ~I_OR_R.m] |= p->x);
    } else if ((insn & I_ADD_R.m) == I_ADD_R.c) {
        // add r
        uint8_t* r = p->regs + (insn & ~I_ADD_R.m);
        uint8_t res = *r + p->x;
        *r = res & 0xF;
        set_zc(p, res);
    } else if ((insn & I_ADC_R.m) == I_ADC_R.c) {
        // adc r
        uint8_t* r = p->regs + (insn & ~I_ADC_R.m);
        uint8_t res = *r + p->x;
        if (p->s & S_C)
            ++res;
        *r = res & 0xF;
        if (p->s & S_Z)
            set_zc(p, res);
        else
            set_c(p, res);
    } else if ((insn & I_SUB_R.m) == I_SUB_R.c) {
        // sub r
        uint8_t* r = p->regs + (insn & ~I_SUB_R.m);
        uint8_t res = *r + ((~p->x + 1) & 0xF);
        *r = res & 0xF;
        set_zc(p, res);
    } else if ((insn & I_SBC_R.m) == I_SBC_R.c) {
        // sbc r
        uint8_t* r = p->regs + (insn & 0x0F);
        uint8_t res = *r + ((~p->x + 1) & 0xFF);
        if ((insn & 0x10) && !(p->s & S_C))
            ++res;
        *r = res & 0xF;
        if (p->s & S_Z)
            set_zc(p, res);
        else
            set_c(p, res);
    } else if ((insn & I_LDI.m) == I_LDI.c) {
        // mov x, k
        p->x = insn & ~I_LDI.m;
    } else if ((insn & I_BR_K.m) == I_BR_K.c) {
        // br k
        uint8_t k = insn & ~I_BR_K.m;
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
    uint8_t* dmem = mmap(
        NULL, 1<<24, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0);
    if (dmem == MAP_FAILED)
        fatal_e(E_RARE, "Can't mmap data memory");

    uint8_t pmem[] = {
        I_LDI.c | 0x02,
        I_STR.c | 0x01,
        I_LDI.c | 0x00,
        I_STR.c | 0x00,
        I_LDI.c | 0x0A,
        I_STM_2R.c | 0x00,
        I_BR_K.c | 0x1F,
    };

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
