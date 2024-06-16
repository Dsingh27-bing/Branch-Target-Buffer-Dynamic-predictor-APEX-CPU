# APEX Pipeline Simulator v2.0
A template for 5 Stage APEX In-order Pipeline

## Notes:

 - This code is a simple implementation template of a working 5-Stage APEX In-order Pipeline
 - Implementation is in `C` language
 - Stages: Fetch -> Decode -> Execute -> Memory -> Writeback
 - You can read, modify and build upon given code-base to add other features as required in project description
 - You are also free to write your own implementation from scratch
 - All the stages have latency of one cycle
 - There is a single functional unit in Execute stage which perform all the arithmetic and logic operations
 - Logic to check data dependencies has not be included
 - Includes logic for `ADD`, `LOAD`, `BZ`, `BNZ`,  `MOVC` and `HALT` instructions
 - On fetching `HALT` instruction, fetch stage stop fetching new instructions
 - When `HALT` instruction is in commit stage, simulation stops
 - You can modify the instruction semantics as per the project description

## Files:

 - `Makefile`
 - `file_parser.c` - Functions to parse input file
 - `apex_cpu.h` - Data structures declarations
 - `apex_cpu.c` - Implementation of APEX cpu
 - `apex_macros.h` - Macros used in the implementation
 - `main.c` - Main function which calls APEX CPU interface
 - `input.asm` - Sample input file

## How to compile and run

 Go to terminal, `cd` into project directory and type:
```
 make
```
 Run as follows:
```
 ./apex_sim <input_file_name>
```

## Author

 - Copyright (C) Gaurav Kothari (gkothar1@binghamton.edu)
 - State University of New York, Binghamton

## Bugs

 - Please contact your TAs for any assistance or query
 - Report bugs at: gkothar1@binghamton.edu