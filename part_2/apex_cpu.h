/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
#define TAKEN 1
#define NOT_TAKEN 0

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int imm;
} APEX_Instruction;

// condition code struct
typedef struct condition_code
{
    int z;
    int p;
    int n;
} condition_code;

typedef struct forward_bus
{
    int reg;
	int value;
}forward_bus;

typedef struct BTB  // stack max 4 enteries
{
    int inst_address;
    int executed; // if it was taken and executed for the first time
    int target_address;
    int prediction_state; // Variable to store the prediction state
    int history_state; //prediction history 
    int num_executed; //number of times branch was executed irrespective of taken,  not taken
} BTB;

// Define the circular queue structure
struct CircularQueue {
    BTB data[4];
    int head, tail;
    unsigned int size;
};

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rs1_f;   //flag if rs1 is found
    int rs2_f;   // flag rs2 is found
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int memory_address;
    int has_insn;
    int stalled;
    int btb_searched;
    int type_of_branch;
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */
    int regs_writing[REG_FILE_SIZE];//for knowing which register is writing currently
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int simulate;
    int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    condition_code cc;            /*condition code flag for cmp,cml {1 (+), 0 (-)}*/
    forward_bus fb;
    struct forward_bus ex_fb;  //excution stage forward bus
    struct forward_bus mem_fb; //memory stage forward bus
    int mem_address[DATA_MEMORY_SIZE];
    int data_counter;
    BTB btb; 
    struct CircularQueue btb_queue; // Circular Queue for BTB entries

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
void simulate_cpu_for_cycles(APEX_CPU *cpu, int num_cycles);
void initCircularQueue(struct CircularQueue *queue);
#endif
