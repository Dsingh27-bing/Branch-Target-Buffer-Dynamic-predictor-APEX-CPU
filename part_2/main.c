/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;

    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if (argc != 2 && (argc != 4 || strcmp(argv[2], "simulate") != 0))
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
        fprintf(stderr, "  To run the code: %s <input_file>\n", argv[0]);
        fprintf(stderr, "  To simulate with a specific number of cycles: %s <input_file> simulate <num_cycles>\n", argv[0]);
        exit(1);
    }

    cpu = APEX_cpu_init(argv[1]);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }
    if (argc == 4) {
        int num_cycles = atoi(argv[3]);
        if (num_cycles <= 0) {
            fprintf(stderr, "APEX_Error: Invalid number of cycles\n");
            exit(1);
        }
        simulate_cpu_for_cycles(cpu, num_cycles);
    } else {
        APEX_cpu_run(cpu);
    }

    //APEX_cpu_run(cpu);
    APEX_cpu_stop(cpu);
    return 0;
}