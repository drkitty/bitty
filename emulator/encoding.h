#pragma once


#define I_IFC_B 0x00
#define I_IFS_B 0x04

#define I_BR_4R 0x08
#define I_BR_8R 0x0C

#define I_COM 0x0E
#define I_ASR 0x0F

#define I_SL 0x10
#define I_SR 0x11
#define I_RL 0x12
#define I_RR 0x13

#define _I_MOV_2R 0x20
#define I_MOV_X_2R 0x20
#define I_MOV_2R_X 0x21

#define _I_MOV_4R 0x30
#define I_MOV_X_4R 0x30
#define I_MOV_4R_X 0x31

#define _I_MOV_8R 0x32
#define I_MOV_X_8R 0x32
#define I_MOV_8R_X 0x33

#define I_EDEC_8R 0x36

#define I_PROC 0x37

#define I_MOV_X_R 0x40
#define I_MOV_R_X 0x50

#define I_AND_R 0x60
#define I_OR_R 0x70

#define _I_ADD 0x80
#define I_ADD_R 0x80
#define I_ADC_R 0x90

#define _I_SUB 0xA0
#define I_SUB_R 0xA0
#define I_SBC_R 0xB0

#define I_XOR_R 0xC0

#define I_MOV_X_K 0xD0

#define I_BR_K 0xE0
