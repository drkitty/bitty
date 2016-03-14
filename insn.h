#pragma once


enum insn_operand_type {
    OPD_2REG, // block of 2 regs
    OPD_4REG, // block of 4 regs
    OPD_8REG, // block of 8 regs
    OPD_NONE, // (intrinsic insn)
    OPD_BIT,  // bit number
    OPD_REG,  // register number
    OPD_NUM,  // plain number
    OPD_OFS,  // offset
};


struct insn {
    uint8_t c; // opcode
    uint8_t m; // opcode mask
    uint8_t t; // operand type
};


#define INSN(opcode, mask) ((struct insn){ .c = opcode, .m = mask })


#define I_BR_2R INSN(0x00, 0xF1)
#define I_BR_4R INSN(0x01, 0xF3)
#define I_BR_8R INSN(0x03, 0xF7)

#define I_COM INSN(0x07, 0xFF)
#define I_ASR INSN(0x0F, 0xFF)

#define I_SL INSN(0x10, 0xFF)
#define I_SR INSN(0x11, 0xFF)
#define I_RL INSN(0x12, 0xFF)
#define I_RR INSN(0x13, 0xFF)

#define I_IFC_B INSN(0x18, 0xFC)
#define I_IFS_B INSN(0x1C, 0xFC)

#define I_LDM_2R INSN(0x20, 0xF1)
#define I_STM_2R INSN(0x21, 0xF1)

#define I_LDM_4R INSN(0x30, 0xF3)
#define I_STM_4R INSN(0x32, 0xF3)

#define I_LDM_8R INSN(0x31, 0xF7)
#define I_STM_8R INSN(0x35, 0xF7)

#define I_EDEC_8R INSN(0x33, 0xF7)

#define I_PROC INSN(0x37, 0xFF)

#define I_LDR INSN(0x40, 0xF0)
#define I_STR INSN(0x50, 0xF0)

#define I_AND_R INSN(0x60, 0xF0)
#define I_OR_R INSN(0x70, 0xF0)

#define I_ADD_R INSN(0x80, 0xF0)
#define I_ADC_R INSN(0x90, 0xF0)

#define I_SUB_R INSN(0xA0, 0xF0)
#define I_SBC_R INSN(0xB0, 0xF0)

#define I_XOR_R INSN(0xC0, 0xF0)

#define I_LDI INSN(0xD0, 0xF0)

#define I_BR_K INSN(0xE0, 0xE0)
