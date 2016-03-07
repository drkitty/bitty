#pragma once


#include "common.h"

#include <stdint.h>

#include "insn.h"


struct line {
    const char* label;
    const char* filename;
    int num;

    uint8_t opc;
    uint8_t opd;
};
