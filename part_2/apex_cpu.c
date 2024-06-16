/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
//final dimple 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

typedef struct {
    int prediction_state;
    int history_state;
} PredictionResult;

void predict_and_update_btb(BTB *btb_entry, int outcome, int type, PredictionResult *result) {
    
    int prediction_state = btb_entry->prediction_state;
    int history_prediction = btb_entry->history_state;
    /* Update prediction state based on the outcome and type */
    if (outcome == TAKEN && type == 0) //bnz bp
    {
        if (history_prediction == 2)
            history_prediction = 3;
        else if (history_prediction == 1)
            history_prediction = 2;
        else if (history_prediction == 3)
            history_prediction = 3;
        else if (history_prediction == 0)
            history_prediction = 1;
    } 
    else if (outcome == NOT_TAKEN && type == 0) 
    {
        if (history_prediction == 1)
            history_prediction = 0;
        else if (history_prediction == 3)
            history_prediction = 2;
        else if (history_prediction == 0)
            history_prediction = 0;
        else if (history_prediction == 2)
            history_prediction = 1;
    } 
    else if (outcome == TAKEN && type == 1) // bz bnp
    {
        if (history_prediction == 2)
            history_prediction = 3;
        else if (history_prediction == 1)
            history_prediction = 2;
        else if (history_prediction == 3)
            history_prediction = 3;
        else if (history_prediction == 0)
            history_prediction = 1;
        
    } 
    else if (outcome == NOT_TAKEN && type == 1) 
    {
        if (history_prediction == 1)
            history_prediction = 0;
        else if (history_prediction == 3)
            history_prediction = 2;
        else if (history_prediction == 0)
            history_prediction = 0;
        else if (history_prediction == 2)
            history_prediction = 1;
    }

    if((history_prediction == 0 || history_prediction == 1 ) && type == 1) //bz bnp
    {
        prediction_state = 0;
    }
    else if((history_prediction == 3 || history_prediction == 2) && type == 1)
    {
        prediction_state = 1;
    }
    else if((history_prediction == 3  || history_prediction == 2) && type == 0) //bnz bp
    {
        prediction_state = 1;
    }
    else if((history_prediction == 0 || history_prediction == 1) && type == 0)
    {
        prediction_state = 0;
    }

    /* Update the BTB entry with the new prediction state */
    btb_entry->prediction_state = prediction_state;
    btb_entry->history_state = history_prediction;

    result->prediction_state = prediction_state;
    result->history_state = history_prediction;

}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_JALR:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }


        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        case OPCODE_LOADP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STOREP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BP:
        case OPCODE_BNP:
        case OPCODE_BN:
        case OPCODE_BNN:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }


        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }
        case OPCODE_CML:
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d", stage->opcode_str,stage->rs1,stage->imm);
            break;
        }
        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d", stage->opcode_str,stage->rs1,stage->rs2);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{

    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n \n \n");
    printf("----------\n%s\n----------\n", "FLAGS (+,-,0):");
    printf("\n Zero flag = %d", cpu->cc.z);
    printf("\n Positive flag = %d", cpu->cc.p);
    printf("\n Negative flag = %d", cpu->cc.n);
    printf("\n");
    printf("\n \n \n");
    printf("----------\n%s\n----------\n", "MEMORY:");
    for (int i =0; i<cpu->data_counter; ++i){
        printf("\nMemory[%d]= %d\n", cpu->mem_address[i],cpu->data_memory[cpu->mem_address[i]]);
    }
    printf("\n\n");
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */

static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;
    if (cpu->fetch.has_insn)
    {    
        cpu->fetch.btb_searched = 0;
        
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE && cpu->fetch.btb_searched ==0)
        {
            
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;
        if(cpu->fetch.stalled == 0 )
        {  
        
          /* Index into code memory using this pc and copy all instruction fields
           * into fetch latch  */
          current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
          strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
          cpu->fetch.opcode = current_ins->opcode;
          cpu->fetch.rd = current_ins->rd;
          cpu->fetch.rs1 = current_ins->rs1;
          cpu->fetch.rs2 = current_ins->rs2;
          cpu->fetch.imm = current_ins->imm;
          for (int i = 0; i < cpu->btb_queue.size; i++) 
            {   
                int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;
                if (cpu->pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->fetch.type_of_branch== 0 && cpu->btb_queue.data[index].prediction_state == 1 && cpu->btb_queue.data[index].target_address != 0) 
                {   
                    
                    
                    cpu->fetch.btb_searched = 1;
                    cpu->pc = cpu->btb_queue.data[index].target_address;
                    cpu->decode = cpu->fetch; //sending branch to decode so that fetch is updated to target address

                    if(cpu->fetch.stalled == 0)
                    {

                        /* Stop fetching new instructions if HALT is fetched */
                        if (cpu->fetch.opcode == OPCODE_HALT)
                        {
                            cpu->fetch.has_insn = FALSE;
                        }
                    }
                    if (ENABLE_DEBUG_MESSAGES)
                    {
                        print_stage_content("Fetch", &cpu->fetch);
                    }

                    return;
                
                }
                else if (cpu->pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->fetch.type_of_branch== 1 && cpu->btb_queue.data[index].prediction_state == 1 && cpu->btb_queue.data[index].target_address != 0) 
                {   
                    
                    cpu->fetch.btb_searched = 1;
                    cpu->pc = cpu->btb_queue.data[index].target_address;
                    cpu->decode = cpu->fetch; //sending branch to decode so that fetch is updated to target address
                    
                    if(cpu->fetch.stalled == 0)
                    {

                        /* Stop fetching new instructions if HALT is fetched */
                        if (cpu->fetch.opcode == OPCODE_HALT)
                        {
                            cpu->fetch.has_insn = FALSE;
                        }
                    }
                    if (ENABLE_DEBUG_MESSAGES)
                    {
                        print_stage_content("Fetch", &cpu->fetch);
                    }

                    return;
                
                }

                
            }
        }
            
        if(cpu->fetch.stalled == 0 && cpu->fetch.btb_searched==0)
        {
            /* Update PC for next instruction */
            cpu->pc += 4;

            /* Copy data from fetch latch to decode latch*/

              cpu->decode = cpu->fetch;
        }

        if(cpu->fetch.stalled == 0)
        {

            /* Stop fetching new instructions if HALT is fetched */
            if (cpu->fetch.opcode == OPCODE_HALT)
            {
                cpu->fetch.has_insn = FALSE;
            }
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
        print_stage_content("Fetch", &cpu->fetch);
        }
    }
    
    
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {


        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {

                if(cpu->decode.rs1== cpu->ex_fb.reg && cpu->decode.rs1_f==0)
                {
                    cpu->decode.rs1_value=cpu->ex_fb.value;
                    cpu->decode.rs1_f =1;
                }
                if(cpu->decode.rs1==cpu->mem_fb.reg && cpu->decode.rs1_f==0)
                {
                    cpu->decode.rs1_value=cpu->mem_fb.value;
                    cpu->decode.rs1_f =1;
                }

                if (cpu->decode.rs1_f==0 && cpu->regs_writing[cpu->decode.rs1] == 0){
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs1_f = 1;
                }
                // ---------------- rs1 done by now
                if( cpu->decode.rs2== cpu->ex_fb.reg && cpu->decode.rs2_f==0)
                {
                    cpu->decode.rs2_value=cpu->ex_fb.value;
                    cpu->decode.rs2_f =1;

                }
                if(cpu->regs_writing[cpu->decode.rs2]==1 && cpu->decode.rs2== cpu->mem_fb.reg && cpu->decode.rs2_f==0 )
                {
                    cpu->decode.rs2_value=cpu->mem_fb.value;
                    cpu->decode.rs2_f =1;
                }
                if(cpu->regs_writing[cpu->decode.rs2] ==0 && cpu->decode.rs2_f==0)
                {
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    cpu->decode.rs2_f = 1;
                }

                if (cpu->decode.rs1_f==1 && cpu->decode.rs2_f==1)
                {
                    cpu->regs_writing[cpu->decode.rd] = 1;
                    cpu->decode.stalled = 0;
                    break;
                }

                else{
                  cpu->decode.stalled = 1;
                  break;
                }
            }
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_JALR:
            case OPCODE_LOAD:
            {
              

                if (cpu->decode.rs1== cpu->ex_fb.reg && cpu->decode.rs1_f ==0 ){
                    cpu->decode.rs1_value = cpu->ex_fb.value;
                    cpu->decode.rs1_f = 1;
                  }

                if (cpu->decode.rs1== cpu->mem_fb.reg && cpu->decode.rs1_f ==0 ){
                    cpu->decode.rs1_value = cpu->mem_fb.value;
                    cpu->decode.rs1_f = 1;
                  }
                  if (cpu->regs_writing[cpu->decode.rs1] == 0 && cpu->decode.rs1_f ==0){
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs1_f = 1;
                    }
                if(cpu->decode.rs1_f){
                    cpu->regs_writing[cpu->decode.rd] = 1;
                    cpu->decode.stalled = 0;
                    break;
                }
                else{
                  cpu->decode.stalled = 1;
                  break;
                }}
            case OPCODE_LOADP:
            {

                if (cpu->decode.rs1== cpu->ex_fb.reg && cpu->decode.rs1_f ==0 ){
                    cpu->decode.rs1_value = cpu->ex_fb.value;
                    cpu->decode.rs1_f = 1;
                    }

                if (cpu->decode.rs1== cpu->mem_fb.reg && cpu->decode.rs1_f ==0){
                    cpu->decode.rs1_value = cpu->mem_fb.value;
                    cpu->decode.rs1_f = 1;
                    }
                if (cpu->regs_writing[cpu->decode.rs1] == 0 && cpu->decode.rs1_f ==0){
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs1_f = 1;
                  }
                if(cpu->decode.rs1_f){
                    cpu->regs_writing[cpu->decode.rs1] = 1;
                    cpu->regs_writing[cpu->decode.rd] = 1;
                    cpu->decode.stalled = 0;
                    break;
                }
                else{
                  cpu->decode.stalled = 1;
                  break;
                }

                }

            case OPCODE_CML:
            case OPCODE_JUMP:
                {

                if (cpu->decode.rs1== cpu->ex_fb.reg && cpu->decode.rs1_f ==0 ){
                    cpu->decode.rs1_value = cpu->ex_fb.value;
                    cpu->decode.rs1_f = 1;}
                if (cpu->decode.rs1== cpu->mem_fb.reg && cpu->decode.rs1_f ==0 ){
                    cpu->decode.rs1_value = cpu->mem_fb.value;
                    cpu->decode.rs1_f = 1;}
                if (cpu->regs_writing[cpu->decode.rs1] == 0 && cpu->decode.rs1_f ==0 ){
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs1_f = 1;
                  }
                if(cpu->decode.rs1_f){
                    cpu->regs_writing[cpu->decode.rs1]=1;
                    cpu->decode.stalled = 0;
                    break;
                }

                else{
                  cpu->decode.stalled = 1;
                  break;
                }

                }

            case OPCODE_STORE:
            {
                if( cpu->decode.rs1 == cpu->ex_fb.reg && cpu->decode.rs1_f ==0)
                {
                    cpu->decode.rs1_value=cpu->ex_fb.value;
                    cpu->decode.rs1_f =1;
                }
                if(cpu->decode.rs2== cpu->ex_fb.reg && cpu->decode.rs2_f ==0)
                {
                    cpu->decode.rs2_value=cpu->ex_fb.value;
                    cpu->decode.rs2_f =1;
                }
                if(cpu->decode.rs1== cpu->mem_fb.reg && cpu->decode.rs1_f ==0)
                {
                    cpu->decode.rs1_value=cpu->mem_fb.value;
                    cpu->decode.rs1_f =1;
                }
                if( cpu->decode.rs2== cpu->mem_fb.reg && cpu->decode.rs2_f ==0)
                {
                    cpu->decode.rs2_value=cpu->mem_fb.value;
                    cpu->decode.rs2_f =1;
                }
                if (cpu->regs_writing[cpu->decode.rs1] == 0 && cpu->decode.rs1_f ==0){
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs1_f = 1;
                }
                if(cpu->regs_writing[cpu->decode.rs2] ==0 && cpu->decode.rs2_f ==0)
                {
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    cpu->decode.rs2_f = 1;
                }
                if (cpu->decode.rs1_f==1 && cpu->decode.rs2_f==1)
                {
                    cpu->decode.stalled = 0;
                    break;
                }
                else{
                  cpu->decode.stalled = 1;
                  break;
                }
            }
            case OPCODE_STOREP:
            {
                if(cpu->decode.rs1== cpu->ex_fb.reg && cpu->decode.rs1_f ==0 )
                {
                    cpu->decode.rs1_value=cpu->ex_fb.value;
                    cpu->decode.rs1_f =1;
                }
                if(cpu->decode.rs2== cpu->ex_fb.reg && cpu->decode.rs2_f ==0  )
                {
                    cpu->decode.rs2_value=cpu->ex_fb.value;
                    cpu->decode.rs2_f =1;
                }
                if(cpu->decode.rs1== cpu->mem_fb.reg && cpu->decode.rs1_f ==0  )
                {
                    cpu->decode.rs1_value=cpu->mem_fb.value;
                    cpu->decode.rs1_f =1;
                }
                if(cpu->decode.rs2== cpu->mem_fb.reg && cpu->decode.rs2_f ==0  )
                {
                    cpu->decode.rs2_value=cpu->mem_fb.value;
                    cpu->decode.rs2_f =1;
                }
                if (cpu->regs_writing[cpu->decode.rs1] == 0 && cpu->decode.rs1_f ==0 ){
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs1_f = 1;
                }
                if(cpu->regs_writing[cpu->decode.rs2] ==0 && cpu->decode.rs2_f ==0 )  // loadp ne pkda hai to ye kaese hua??
                {
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    cpu->decode.rs2_f = 1;
                }

                if (cpu->decode.rs1_f==1 && cpu->decode.rs2_f==1)
                {
                    
                    cpu->decode.stalled = 0;
                    break;
                }
                else{
                  cpu->decode.stalled = 1;
                  break;
                }
            }

            case OPCODE_CMP:
            {


                if(cpu->decode.rs1== cpu->ex_fb.reg && cpu->decode.rs1_f ==0 )
                {
                    cpu->decode.rs1_value=cpu->ex_fb.value;
                    cpu->decode.rs1_f =1;
                }
                if(cpu->decode.rs2== cpu->ex_fb.reg && cpu->decode.rs2_f ==0 )
                {
                    cpu->decode.rs2_value=cpu->ex_fb.value;
                    cpu->decode.rs2_f =1;
                }
                if(cpu->decode.rs1== cpu->mem_fb.reg && cpu->decode.rs1_f ==0 )
                {
                    cpu->decode.rs1_value=cpu->mem_fb.value;
                    cpu->decode.rs1_f =1;
                }
                if(cpu->decode.rs2== cpu->mem_fb.reg && cpu->decode.rs2_f ==0 )
                {
                    cpu->decode.rs2_value=cpu->mem_fb.value;
                    cpu->decode.rs2_f =1;
                }
                if (cpu->regs_writing[cpu->decode.rs1] == 0 && cpu->decode.rs1_f ==0){
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs1_f = 1;
                }
                if(cpu->regs_writing[cpu->decode.rs2] ==0 && cpu->decode.rs2_f ==0)
                {
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    cpu->decode.rs2_f = 1;
                }

                if (cpu->decode.rs1_f==1 && cpu->decode.rs2_f==1)
                {
                    cpu->decode.stalled = 0;
                    break;
                }
                else{
                  cpu->decode.stalled = 1;
                  break;
                }
            }

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                cpu->regs_writing[cpu->decode.rd] = 1;
                break;
            }
            case OPCODE_BNZ:
            case OPCODE_BP:
            {
                
                if (cpu->btb_queue.size < 4 && cpu->decode.btb_searched == 0) 
                {   
                    
                    // The queue is not full, add a new entry
                    int inst_address_exists = 0;
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                    {
                        if (cpu->decode.pc == cpu->btb_queue.data[i].inst_address)
                        {
                            inst_address_exists = 1;
                            break;
                        }
                    }

                    if (!inst_address_exists)
                    {
                        // Add a new entry
                        cpu->btb_queue.data[cpu->btb_queue.tail].inst_address = cpu->decode.pc;
                        cpu->btb_queue.data[cpu->btb_queue.tail].prediction_state =1;
                        cpu->btb_queue.data[cpu->btb_queue.tail].history_state =3;
                        cpu->btb_queue.data[cpu->btb_queue.tail].executed =0;
                        cpu->btb_queue.data[cpu->btb_queue.tail].num_executed=0;
                        cpu->btb_queue.size++;
                        cpu->btb_queue.tail = (cpu->btb_queue.tail + 1) % 4; // Update tail
                    }
                    
                    
                    break;  
                } 
                else if (cpu->btb_queue.size >= 4 && cpu->decode.btb_searched == 0)
                {
                    
                    // The queue is full, replace the oldest entry (FIFO)
                    int inst_address_exists = 0;
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                    {
                        if (cpu->decode.pc == cpu->btb_queue.data[i].inst_address)
                        {
                            inst_address_exists = 1;
                            break;
                        }
                    }
                    if (!inst_address_exists)
                    {
                        // Replace the oldest entry
                        int replaced_index = cpu->btb_queue.head;
                        cpu->btb_queue.data[replaced_index].inst_address = cpu->decode.pc;
                        cpu->btb_queue.data[replaced_index].target_address = 0; // Reset target_address
                        cpu->btb_queue.data[replaced_index].prediction_state = 1; // Reset prediction_state
                        cpu->btb_queue.data[replaced_index].executed = 0;        // Reset executed
                        cpu->btb_queue.data[replaced_index].history_state = 3;
                        cpu->btb_queue.data[replaced_index].num_executed =0;

                        // Update head
                        cpu->btb_queue.head = (cpu->btb_queue.head + 1) % 4;
                    }
                            
                    break;
                    
                }
             
            }
            case OPCODE_BNP:
            case OPCODE_BZ:
            {
                
                 
                if (cpu->btb_queue.size < 4 && cpu->decode.btb_searched == 0) 
                {   
                    
                    // The queue is not full, add a new entry
                    int inst_address_exists = 0;
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                    {
                        if (cpu->decode.pc == cpu->btb_queue.data[i].inst_address)
                        {
                            inst_address_exists = 1;
                            break;
                        }
                    }

                    if (!inst_address_exists)
                    {
                        // Add a new entry
                        cpu->btb_queue.data[cpu->btb_queue.tail].inst_address = cpu->decode.pc;
                        cpu->btb_queue.data[cpu->btb_queue.tail].prediction_state =0;
                        cpu->btb_queue.data[cpu->btb_queue.tail].history_state =0;
                        cpu->btb_queue.data[cpu->btb_queue.tail].executed =0;
                        cpu->btb_queue.data[cpu->btb_queue.tail].num_executed=0;
                        cpu->btb_queue.size++;
                        cpu->btb_queue.tail = (cpu->btb_queue.tail + 1) % 4; // Update tail
                    }
                    
                    break;  
                } 
                else if (cpu->btb_queue.size >= 4 && cpu->decode.btb_searched == 0)
                {
                    
                   
                    // The queue is full, replace the oldest entry (FIFO)
                    int inst_address_exists = 0;
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                    {
                        if (cpu->decode.pc == cpu->btb_queue.data[i].inst_address)
                        {
                            inst_address_exists = 1;
                            break;
                        }
                    }
                    if (!inst_address_exists)
                    {
                        // Replace the oldest entry
                        int replaced_index = cpu->btb_queue.head;
                        cpu->btb_queue.data[replaced_index].inst_address = cpu->decode.pc;
                        cpu->btb_queue.data[replaced_index].target_address = 0; // Reset target_address
                        cpu->btb_queue.data[replaced_index].prediction_state = 0; // Reset prediction_state
                        cpu->btb_queue.data[replaced_index].executed = 0;        // Reset executed
                        cpu->btb_queue.data[replaced_index].history_state = 0;
                        cpu->btb_queue.data[replaced_index].num_executed =0;

                        // Update head
                        cpu->btb_queue.head = (cpu->btb_queue.head + 1) % 4;
                    }
      
                    break;
                    
                }

            }

        }

        /* Copy data from decode latch to execute latch*/

        if (cpu->decode.stalled == 0){
          cpu->execute = cpu->decode;
          cpu->fetch.stalled = 0;
        }
        else{
          cpu->fetch.stalled = 1;
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
    }

}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {

          case OPCODE_ADD:
          {
            if(cpu->regs_writing[cpu->execute.rd] == 0){
              cpu->regs_writing[cpu->execute.rd] = 1;
            }
              cpu->execute.result_buffer
                  = cpu->execute.rs1_value + cpu->execute.rs2_value;
              cpu->ex_fb.reg= cpu->execute.rd;
              cpu->ex_fb.value = cpu->execute.result_buffer;
              /* Set the zero flag based on the result buffer */
              if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }

              break;
          }
          case OPCODE_ADDL:
          {

            if(cpu->regs_writing[cpu->execute.rd] == 0){
              cpu->regs_writing[cpu->execute.rd] = 1;
            }
              cpu->execute.result_buffer
                  = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->ex_fb.reg= cpu->execute.rd;
              cpu->ex_fb.value = cpu->execute.result_buffer;
              /* Set the zero flag based on the result buffer */
              if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }

              break;
          }
          case OPCODE_SUB:
          {
            if(cpu->regs_writing[cpu->execute.rd] == 0){
              cpu->regs_writing[cpu->execute.rd] = 1;
            }
              cpu->execute.result_buffer
                  = cpu->execute.rs1_value - cpu->execute.rs2_value;
                   cpu->ex_fb.reg= cpu->execute.rd;
              cpu->ex_fb.value = cpu->execute.result_buffer;

              /* Set the zero flag based on the result buffer */
             if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }

              break;
          }
          case OPCODE_SUBL:
          {
            if(cpu->regs_writing[cpu->execute.rd] == 0){
              cpu->regs_writing[cpu->execute.rd] = 1;
            }
              cpu->execute.result_buffer
                  = cpu->execute.rs1_value - cpu->execute.imm;
                   cpu->ex_fb.reg= cpu->execute.rd;
              cpu->ex_fb.value = cpu->execute.result_buffer;

              /* Set the zero flag based on the result buffer */
              if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }

              break;
          }
          case OPCODE_MUL:
          {
            if(cpu->regs_writing[cpu->execute.rd] == 0){
              cpu->regs_writing[cpu->execute.rd] = 1;
            }
              cpu->execute.result_buffer
                  = cpu->execute.rs1_value * cpu->execute.rs2_value;
             cpu->ex_fb.reg= cpu->execute.rd;
              cpu->ex_fb.value = cpu->execute.result_buffer;
              /* Set the zero flag based on the result buffer */
              if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }
              break;
          }
          case OPCODE_AND:
          {
            if(cpu->regs_writing[cpu->execute.rd] == 0){
              cpu->regs_writing[cpu->execute.rd] = 1;
            }

              cpu->execute.result_buffer
                  = cpu->execute.rs1_value&cpu->execute.rs2_value;
                 cpu->ex_fb.reg= cpu->execute.rd;
              cpu->ex_fb.value = cpu->execute.result_buffer;
              /* Set the zero flag based on the result buffer */
              if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }

              break;
          }
          case OPCODE_OR:
          {
            if(cpu->regs_writing[cpu->execute.rd] == 0){
              cpu->regs_writing[cpu->execute.rd] = 1;
            }
              cpu->execute.result_buffer
                  = cpu->execute.rs1_value | cpu->execute.rs2_value;
                  cpu->ex_fb.reg= cpu->execute.rd;
              cpu->ex_fb.value = cpu->execute.result_buffer;
              /* Set the zero flag based on the result buffer */
              if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }

              break;
          }
          case OPCODE_XOR:
          {
            if(cpu->regs_writing[cpu->execute.rd] == 0){
              cpu->regs_writing[cpu->execute.rd] = 1;
            }
              cpu->execute.result_buffer = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
              cpu->ex_fb.reg= cpu->execute.rd;
              cpu->ex_fb.value = cpu->execute.result_buffer;
              /* Set the zero flag based on the result buffer */
              if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }

              break;
          }
            case OPCODE_LOAD:
            {

              if(cpu->regs_writing[cpu->execute.rd] == 0){
                cpu->regs_writing[cpu->execute.rd] = 1;
              }
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }
            case OPCODE_LOADP:
            {

              // if regs_writing was over-written

              if(cpu->regs_writing[cpu->execute.rd] == 0){
                cpu->regs_writing[cpu->execute.rd] = 1;
              }
              if(cpu->regs_writing[cpu->execute.rs1] == 0){
                cpu->regs_writing[cpu->execute.rs1] = 1;
              }

                cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->execute.rs1_value=cpu->execute.rs1_value+4;
                cpu->ex_fb.reg = cpu->execute.rs2;
                cpu->ex_fb.value = cpu->execute.rs2_value;
                break;
            }
            case OPCODE_STORE:
            {

                cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
                cpu->mem_address[cpu->data_counter++] = cpu->execute.memory_address;
                break;
            }
            case OPCODE_STOREP:
            {
               
              if(cpu->regs_writing[cpu->execute.rs2] == 0){
                cpu->regs_writing[cpu->execute.rs2] = 1;
              }
                cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
                cpu->execute.rs2_value =cpu->execute.rs2_value +4;
                cpu->ex_fb.reg = cpu->execute.rs2;
                cpu->ex_fb.value = cpu->execute.rs2_value;
                cpu->mem_address[cpu->data_counter++] = cpu->execute.memory_address;
                break;
            }
            case OPCODE_BZ:
            {
                
                cpu->execute.type_of_branch=1;
             
                if (cpu->zero_flag == TRUE)
                {
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                            {
                                int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;

                                if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].executed == 0 && cpu->btb_queue.data[index].prediction_state == 0 && cpu->btb_queue.data[index].num_executed==0)
                                {
                                    //first time condition is true
                                    /* Calculate new PC, and send it to fetch unit */
                                    cpu->pc = cpu->execute.pc + cpu->execute.imm;

                                    cpu->btb_queue.data[index].executed = 1;
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Update BTB entry with the correct target address */
                                    cpu->btb_queue.data[index].target_address = cpu->pc;

                                    /* Predict the outcome and update BTB entry */
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    
                                    cpu->btb_queue.data[index].history_state = result.history_state;

                                    /* Since we are using reverse callbacks for pipeline stages,
                                    * this will prevent the new instruction from being fetched in the current cycle*/
                                    cpu->fetch_from_next_cycle = TRUE;

                                    /* Flush previous stages */
                                    cpu->decode.has_insn = FALSE;

                                    /* Make sure fetch stage is enabled to start fetching from the new PC */
                                    cpu->fetch.has_insn = TRUE;

                                    break;
                                }
                                else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 0)
                                    {
                                        //second time it is running with true
                                        /* Calculate new PC, and send it to fetch unit */
                                        cpu->pc = cpu->execute.pc + cpu->execute.imm;
                                        cpu->btb_queue.data[index].num_executed ++;
                                        /* Update BTB entry with the correct target address */
                                        cpu->btb_queue.data[index].target_address = cpu->pc;
                                        /* Predict the outcome and update BTB entry */
                                        PredictionResult result;
                                        predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                        cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                        cpu->btb_queue.data[index].history_state = result.history_state;
                                        
                                        /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;

                                        /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;

                                        /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        break;
                                    }
                                else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 1 )
                                    {
                                        // second time onwards running for true
                                        cpu->btb_queue.data[index].num_executed ++;
                                        PredictionResult result;
                                        predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                        cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                        cpu->btb_queue.data[index].history_state = result.history_state;
                                        cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                        
                                        break;
                                    }
    
                            }
                    
                    break; 
                } 
        
                
                else if (cpu->zero_flag == FALSE)
                {
                   
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                        {
                            int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;

                            if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed >0 && cpu->btb_queue.data[index].prediction_state == 1)
                                {
                                    
                                    //after it was run before for true now it is false again
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;

                                    //write logic for flushing everything                                  
                                    /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;
                                    /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;
                                    /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        cpu->pc = cpu->execute.pc +4;
                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed == 0 && cpu->btb_queue.data[index].prediction_state == 0)
                                {
                                    
                                    // first time and false condition
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    break;
                                }
                            else if(cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 0)
                                {
                                    
                                    // second onwards time and false condition
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    break;
                                }
                        }
                        break;
                }
      
                break;
            }

            case OPCODE_BNZ:
            {
                cpu->execute.type_of_branch=0;
                
                if (cpu->zero_flag == FALSE)
                {
                   
                        /* Try to find the BTB entry with matching inst_address */
                        for (int i = 0; i < cpu->btb_queue.size; i++)
                            {
                                int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;

                                if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].executed == 0 && cpu->btb_queue.data[index].num_executed==0)
                                {
                                    // first time runnig when condition is true 
                                    /* Calculate new PC, and send it to fetch unit */
                                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                                    cpu->btb_queue.data[index].num_executed ++;
                                    cpu->btb_queue.data[index].executed = 1;

                                    /* Update BTB entry with the correct target address */
                                    cpu->btb_queue.data[index].target_address = cpu->pc;

                                    /* Predict the outcome and update BTB entry */
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;

                                    /* Since we are using reverse callbacks for pipeline stages,
                                    * this will prevent the new instruction from being fetched in the current cycle*/
                                    cpu->fetch_from_next_cycle = TRUE;

                                    /* Flush previous stages */
                                    cpu->decode.has_insn = FALSE;

                                    /* Make sure fetch stage is enabled to start fetching from the new PC */
                                    cpu->fetch.has_insn = TRUE;

                                    break;
                                }
                                else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state ==0)
                                    {
                                        // it has run multiple time and been false before now it is runnning again
        
                                        /* Calculate new PC, and send it to fetch unit */
                                        cpu->pc = cpu->execute.pc + cpu->execute.imm;
                                        cpu->btb_queue.data[index].num_executed ++;
                                        /* Update BTB entry with the correct target address */
                                        cpu->btb_queue.data[index].target_address = cpu->pc;
                                        /* Predict the outcome and update BTB entry */
                                        PredictionResult result;
                                        predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                        cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                        cpu->btb_queue.data[index].history_state = result.history_state;

                                        /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;

                                        /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;

                                        /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        break;
                                    }
                                else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 1 )
                                    {
                                        //when it was running with everything true and prediction is also right
                                        cpu->btb_queue.data[index].num_executed ++;
                                        PredictionResult result;
                                        predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                        cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                        cpu->btb_queue.data[index].history_state = result.history_state;
                                        cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm ;

                                        break;
                                    }
    
                                
                            }
                        break;  
                }
                
                else if (cpu->zero_flag == TRUE )
                {
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                        {
                            int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;

                            if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 1)
                                {
                                   
                                    //after many trues false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc +cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    //write logic for flushing everything                                  
                                    /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;
                                    /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;
                                    /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        cpu->pc = cpu->execute.pc +4;
                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 &&cpu->btb_queue.data[index].prediction_state == 0 && cpu->btb_queue.data[index].history_state < 3)
                                {
                                    
                                    // third onwards time coming and it is still false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed == 1 &&cpu->btb_queue.data[index].prediction_state == 1 && cpu->btb_queue.data[index].history_state <= 3)
                                {
                                    
                                    //second time it is false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    //write logic for flushing everything                                  
                                    /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;
                                    /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;
                                    /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        cpu->pc = cpu->execute.pc +4;
                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed == 0 &&cpu->btb_queue.data[index].prediction_state == 1 && cpu->btb_queue.data[index].history_state <= 3)
                                {
                                   
                                    //first time it is false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    break;
                                }
                        }
                    
                    break;
                }
                
            }
            case OPCODE_BN:
            {
                if (cpu->cc.n == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BNN:
            {
                if (cpu->cc.n == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BP:
            {
                cpu->execute.type_of_branch=0;
                
                if (cpu->cc.p == TRUE)
                {
                    
                       for (int i = 0; i < cpu->btb_queue.size; i++)
                            {
                                int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;
                                
                                if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].executed == 0 && cpu->btb_queue.data[index].prediction_state ==1 && cpu->btb_queue.data[index].num_executed==0)
                                {
                                    // first time true
                                    cpu->btb_queue.data[index].executed =1;
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Calculate new PC, and send it to fetch unit */
                                    cpu->pc = cpu->execute.pc + cpu->execute.imm;

                                    /* Update BTB entry with the correct target address */
                                    cpu->btb_queue.data[index].target_address = cpu->pc;

                                    /* Predict the outcome and update BTB entry */
                                    //cpu->btb_queue.data[index].prediction_state = predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch);
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;

                                    /* Since we are using reverse callbacks for pipeline stages,
                                    * this will prevent the new instruction from being fetched in the current cycle*/
                                    cpu->fetch_from_next_cycle = TRUE;

                                    /* Flush previous stages */
                                    cpu->decode.has_insn = FALSE;

                                    /* Make sure fetch stage is enabled to start fetching from new PC */
                                    cpu->fetch.has_insn = TRUE;

                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 0 )
                                    {
                                        // after many false true again
                                        /* Calculate new PC, and send it to fetch unit */
                                        cpu->pc = cpu->execute.pc + cpu->execute.imm;
                                        cpu->btb_queue.data[index].num_executed ++;
                                        /* Update BTB entry with the correct target address */
                                        cpu->btb_queue.data[index].target_address = cpu->pc;
                                        /* Predict the outcome and update BTB entry */
                                        //cpu->btb_queue.data[index].prediction_state = predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch);
                                        PredictionResult result;
                                        predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                        cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                        cpu->btb_queue.data[index].history_state = result.history_state;

                                        /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;

                                        /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;

                                        /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        break;
                                    }
                                else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 1 )
                                    {
                                        
                                        cpu->btb_queue.data[index].num_executed ++;
                                        cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                        //cpu->btb_queue.data[index].prediction_state = predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch);
                                        PredictionResult result;
                                        predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                        cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                        cpu->btb_queue.data[index].history_state = result.history_state;

                                        break;
                                    }
                                
                                
                            }
                        break;
                }
                
                
                
                else if (cpu->cc.p == FALSE)
                {
                    
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                        {
                            int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;

                            if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed == 1 &&cpu->btb_queue.data[index].prediction_state == 1)
                                {
                                    //second time false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;

                                    //write logic for flushing everything 
                                    /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;
                                    /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;
                                    /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        cpu->pc = cpu->execute.pc +4;
                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed == 0 &&cpu->btb_queue.data[index].prediction_state == 1 && cpu->btb_queue.data[index].history_state <= 3)
                                {
                                    
                                    //first time it is false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 &&cpu->btb_queue.data[index].prediction_state == 0)
                                {
                                    //third time onwards and still false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 &&cpu->btb_queue.data[index].prediction_state == 1)
                                {
                                    //after many trues false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    //write logic for flushing everything                                  
                                    /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;
                                    /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;
                                    /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        cpu->pc = cpu->execute.pc +4;
                                    break;
                                }
                        }
                    break;
                
                }
            }
            case OPCODE_BNP:
            {
                cpu->execute.type_of_branch=1;
                
                if (cpu->cc.p == FALSE)
                {
                   
                       for (int i = 0; i < cpu->btb_queue.size; i++)
                            {
                                int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;
                                
                                if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].executed == 0 && cpu->btb_queue.data[index].prediction_state == 0 && cpu->btb_queue.data[index].num_executed == 0)
                                {
                                    //first time true
                                    cpu->btb_queue.data[index].executed =1;
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Calculate new PC, and send it to fetch unit */
                                    cpu->pc = cpu->execute.pc + cpu->execute.imm;

                                    /* Update BTB entry with the correct target address */
                                    cpu->btb_queue.data[index].target_address = cpu->pc;

                                    /* Predict the outcome and update BTB entry */
                                    //cpu->btb_queue.data[index].prediction_state = predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch);
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;

                                    /* Since we are using reverse callbacks for pipeline stages,
                                    * this will prevent the new instruction from being fetched in the current cycle*/
                                    cpu->fetch_from_next_cycle = TRUE;

                                    /* Flush previous stages */
                                    cpu->decode.has_insn = FALSE;

                                    /* Make sure fetch stage is enabled to start fetching from new PC */
                                    cpu->fetch.has_insn = TRUE;

                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 0 )
                                    {
                                        //second time true and for true which come after many false
                                        cpu->btb_queue.data[index].num_executed ++;
                                        /* Calculate new PC, and send it to fetch unit */
                                        cpu->pc = cpu->execute.pc + cpu->execute.imm;
                                        /* Update BTB entry with the correct target address */
                                        cpu->btb_queue.data[index].target_address = cpu->pc;
                                        /* Predict the outcome and update BTB entry */
                                        PredictionResult result;
                                        predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                        cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                        cpu->btb_queue.data[index].history_state = result.history_state;

                                        /* Since we are using reverse callbacks for pipeline stages,
                                        * this will prevent the new instruction from being fetched in the current cycle*/
                                        cpu->fetch_from_next_cycle = TRUE;

                                        /* Flush previous stages */
                                        cpu->decode.has_insn = FALSE;

                                        /* Make sure fetch stage is enabled to start fetching from new PC */
                                        cpu->fetch.has_insn = TRUE;
                                        break;
                                    }
                                else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].num_executed > 0 && cpu->btb_queue.data[index].prediction_state == 1 )
                                    {
                                        //3 onwards and still true
                                        cpu->btb_queue.data[index].num_executed ++;
                                        cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm ;
                                        PredictionResult result;
                                        predict_and_update_btb(&cpu->btb_queue.data[index], 1, cpu->execute.type_of_branch, &result);
                                        cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                        cpu->btb_queue.data[index].history_state = result.history_state;

                                        break;
                                    }
                                
                            }
                        
                            break;
                        
                        
                }
                
                else if (cpu->cc.p == TRUE) 
                {
                    
                    for (int i = 0; i < cpu->btb_queue.size; i++)
                        {
                            int index = (cpu->btb_queue.head + i) % cpu->btb_queue.size;

                            if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].prediction_state ==1 && cpu->btb_queue.data[index].num_executed > 0)
                                {
                                    
                                    //after many trues false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    //write logic for flushing everything 
                                    /* Since we are using reverse callbacks for pipeline stages,
                                    * this will prevent the new instruction from being fetched in the current cycle*/
                                    cpu->fetch_from_next_cycle = TRUE;
                                    /* Flush previous stages */
                                    cpu->decode.has_insn = FALSE;
                                    /* Make sure fetch stage is enabled to start fetching from new PC */
                                    cpu->fetch.has_insn = TRUE;
                                    cpu->pc = cpu->execute.pc +4;
                                    break;
                                }
                            else if (cpu->execute.pc == cpu->btb_queue.data[index].inst_address && cpu->btb_queue.data[index].prediction_state == 0 && cpu->btb_queue.data[index].num_executed >= 0)
                                {
                                    
                                    //first onwards and contious to be false
                                    cpu->btb_queue.data[index].num_executed ++;
                                    /* Predict the outcome and update BTB entry */
                                    cpu->btb_queue.data[index].target_address = cpu->execute.pc + cpu->execute.imm;
                                    //cpu->btb_queue.data[index].prediction_state = predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch);
                                    PredictionResult result;
                                    predict_and_update_btb(&cpu->btb_queue.data[index], 0, cpu->execute.type_of_branch, &result);
                                    cpu->btb_queue.data[index].prediction_state = result.prediction_state;
                                    cpu->btb_queue.data[index].history_state = result.history_state;
                                    break;
                                }
                        }
                        break;
                
                }
                
            }
            case OPCODE_JUMP:
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages,
                    * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;

                break;
            }
            case OPCODE_JALR:
            {   
                if(cpu->regs_writing[cpu->memory.rd] == 0){
                  cpu->regs_writing[cpu->memory.rd] = 1;
                }

                /* Calculate new PC, and send it to fetch unit */
                cpu->execute.result_buffer = cpu->execute.pc + 4;
                cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;


                /* Since we are using reverse callbacks for pipeline stages,
                    * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;

                break;
            }

            case OPCODE_MOVC:
            {
              if(cpu->regs_writing[cpu->execute.rd] == 0){
                cpu->regs_writing[cpu->execute.rd] = 1;
              }
                cpu->execute.result_buffer = cpu->execute.imm + 0;
                cpu->ex_fb.reg= cpu->execute.rd;
                cpu->ex_fb.value = cpu->execute.result_buffer;
                break;
            }
            case OPCODE_CML:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value-cpu->execute.imm;

                if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                else if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                else if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }
                break;
            }
            case OPCODE_CMP:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value-cpu->execute.rs2_value;

                if(cpu->execute.result_buffer<0){
                    cpu->cc.p = FALSE;
                    cpu->cc.n = TRUE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }
                else if(cpu->execute.result_buffer>0){
                    cpu->cc.p = TRUE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = FALSE;
                    cpu->zero_flag = FALSE;
                }

                /* Set the zero flag based on the result buffer */
                else if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.p = FALSE;
                    cpu->cc.n = FALSE;
                    cpu->cc.z = TRUE;
                    cpu->zero_flag = TRUE;
                }

                break;
            }
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

         if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }

}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {

                /* No work for ADD */
                if(cpu->regs_writing[cpu->memory.rd] == 0){
                  cpu->regs_writing[cpu->memory.rd] = 1;
                }
                cpu->mem_fb.reg= cpu->memory.rd;
                cpu->mem_fb.value = cpu->memory.result_buffer;
                break;
            }
            case OPCODE_MOVC:{
              if(cpu->regs_writing[cpu->memory.rd] == 0){
                cpu->regs_writing[cpu->memory.rd] = 1;
              }
              cpu->mem_fb.reg= cpu->memory.rd;
              cpu->mem_fb.value = cpu->memory.result_buffer;
                break;
            }

            case OPCODE_LOAD:{
              if(cpu->regs_writing[cpu->memory.rd] == 0){
                cpu->regs_writing[cpu->memory.rd] = 1;
              }
                /* Read from data memory */
                cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
                cpu->mem_fb.reg= cpu->memory.rd;
                cpu->mem_fb.value = cpu->memory.result_buffer;
                break;
            }
            case OPCODE_LOADP:
            {
              if(cpu->regs_writing[cpu->memory.rd] == 0){
                cpu->regs_writing[cpu->memory.rd] = 1;
              }
              if(cpu->regs_writing[cpu->memory.rs1] == 0){
                cpu->regs_writing[cpu->memory.rs1] = 1;
              }
                /* Read from data memory */
                cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
                cpu->mem_fb.reg= cpu->memory.rd;
                cpu->mem_fb.value = cpu->memory.result_buffer;
                break;
            }

            case OPCODE_STORE:
            {

                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address]= cpu->memory.rs1_value;
                // cpu->mem_fb.reg= -1;
                // cpu->mem_fb.value = 0;
                cpu->mem_fb.reg= cpu->memory.rs2;
                cpu->mem_fb.value = cpu->memory.rs2_value;
                break;
            }
            case OPCODE_STOREP:
            {

              if(cpu->regs_writing[cpu->memory.rs2] == 0){
                cpu->regs_writing[cpu->memory.rs2] = 1;
              }

                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address]= cpu->memory.rs1_value;
                // cpu->mem_fb.reg= -1;
                // cpu->mem_fb.value = 0;
                cpu->mem_fb.reg= cpu->memory.rs2;
                cpu->mem_fb.value = cpu->memory.rs2_value;

                break;
            }
            case OPCODE_JALR:
            {
                if(cpu->regs_writing[cpu->memory.rd] == 0){
                  cpu->regs_writing[cpu->memory.rd] = 1;
                }
            }
            case OPCODE_JUMP:
            {
                break;
            }
        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }

}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
          case OPCODE_ADD:
          case OPCODE_ADDL:
          case OPCODE_SUB:
          case OPCODE_SUBL:
          case OPCODE_MUL:
          case OPCODE_AND:
          case OPCODE_OR:
          case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs_writing[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs_writing[cpu->writeback.rd] = 0;

                break;
            }
            case OPCODE_LOADP:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rs1]=cpu->writeback.rs1_value;
                cpu->regs_writing[cpu->writeback.rd] = 0;
                cpu->regs_writing[cpu->writeback.rs1] =0;
                break;
            }
            case OPCODE_STOREP:
            {
                cpu->regs[cpu->writeback.rs2]=cpu->writeback.rs2_value;
                cpu->regs_writing[cpu->writeback.rs2] = 0;
                
                break;
            }

            case OPCODE_MOVC:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs_writing[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_JALR:
            {
                cpu->regs[cpu->writeback.rd]= cpu->writeback.result_buffer;
                cpu->regs_writing[cpu->writeback.rd] = 0;
                break;
            }
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

         if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }
        if (cpu->writeback.opcode == OPCODE_HALT)
            {
                /* Stop the APEX simulator */
                return TRUE;
            }

    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    cpu->data_counter = 0;
    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    memset(&cpu->btb_queue, 0, sizeof(cpu->btb_queue)); // Initialize to all zeros
    cpu->btb_queue.head = 0;
    cpu->btb_queue.tail = 0;
    cpu->btb_queue.size = 0;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void simulate_cpu_for_cycles(APEX_CPU *cpu, int num_cycles) {
    for (int cycle = 1; cycle <= num_cycles; cycle++) {
        if (ENABLE_DEBUG_MESSAGES) {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cycle);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu)) {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cycle, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);

    }
}

void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;
    cpu->clock=1;
    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }
        
        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);
        
        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}
