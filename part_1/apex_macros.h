/*
 * apex_macros.h
 * Contains APEX cpu pipeline macros
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _MACROS_H_
#define _MACROS_H_

#define FALSE 0x0
#define TRUE 0x1

/* Integers */
#define DATA_MEMORY_SIZE 4096

/* Size of integer register file */
#define REG_FILE_SIZE 32

/* Numeric OPCODE identifiers for instructions */
#define OPCODE_NOP 0x0
#define OPCODE_ADD 0x1
#define OPCODE_SUB 0x2
#define OPCODE_MUL 0x3
#define OPCODE_DIV 0x4
#define OPCODE_AND 0x5
#define OPCODE_OR 0x6
#define OPCODE_XOR 0x7
#define OPCODE_MOVC 0x8
#define OPCODE_LOAD 0x9
#define OPCODE_STORE 0xa
#define OPCODE_BZ 0xb
#define OPCODE_BNZ 0xc
#define OPCODE_ADDL 0xe
#define OPCODE_SUBL 0xf
#define OPCODE_LOADP 0x10
#define OPCODE_STOREP 0x11
#define OPCODE_CML 0x12
#define OPCODE_CMP 0x13
#define OPCODE_BP 0x14
#define OPCODE_BNP 0x15
#define OPCODE_BN 0x16
#define OPCODE_BNN 0x17
#define OPCODE_JUMP 0x18
#define OPCODE_JALR 0x19
#define OPCODE_HALT 0x1a



/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/* Set this flag to 1 to enable cycle single-step mode */
#define ENABLE_SINGLE_STEP 1
#define SIMULATE_STEP 0
#endif
