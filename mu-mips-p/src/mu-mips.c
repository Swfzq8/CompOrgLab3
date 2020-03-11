#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

void print_instruction(uint32_t addr){
	
	printf("print_instruction has not been implemented yet\n");

	return;
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_pipeline();
	CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT);
	printf("PC\t: 0x%08x\n", ID_EX.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll(); 
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	ID_EX.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* maintain the pipeline                                                                                           */ 
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	
	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */ 
/************************************************************/
void WB()
{
	/*IMPLEMENT THIS*/
// for register-register instruction: REGS[rd] <= ALUOutput
// for register-immediate instruction: REGS[rt] <= ALUOutput
// for load instruction: REGS[rt] <= LMD
			uint32_t rt = MEM_WB.B;
			uint32_t rd = MEM_WB.rd;  
	if(MEM_WB.opcode == 0x00)
		switch(MEM_WB.funct){
		case 0x20: //ADD
			NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
			break;
		case 0x21: //ADDU 
			NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
			break;
		case 0x22: //SUB
			NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
			break;
		case 0x23: //SUBU
			NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
			break;
		case 0x24: //AND
			NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
			break;
		case 0x25: //OR
			NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
			break;
		case 0x26: //XOR
			NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
			break;
		case 0x27: //NOR
			NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;
			break;
		default:
		printf("operation not implemented yet\n");
		}
	else {
		switch(MEM_WB.opcode){
			
			case 0x20://LB
				NEXT_STATE.REGS[rt] = MEM_WB.LMD;
				break;
			case 0x21://LH
				NEXT_STATE.REGS[rt] = MEM_WB.LMD;
				break;
			case 0x23://LW
				NEXT_STATE.REGS[rt] = MEM_WB.LMD;
				break;
			case 0x08: //ADDI
				NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
				break;
			case 0x09: //ADDIU
				NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
				break;
			case 0x0A: //SLTI
				NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
				break;
			case 0x0C: //ANDI
				NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
				break;
			case 0x0D: //ORI
				NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
				break;
			case 0x0E: //XORI
				NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
				break;
			default:
				printf("operation not implemented yet\n");
		}
	}
	



}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	/*IMPLEMENT THIS
	*/

// for load: LMD <= MEM[ALUOutput]
// for store: MEM[ALUOutput] <= B
// If the instruction is load, data is read from memory and stored in load memory data (LMD) register. If
// it is a store instruction, then the value stored in register B is written into memory. The address of
// memory to be accessed is the value computed in the previous stage and stored in ALUOutput register. 	
	uint32_t data;
	MEM_WB.B = ID_EX.A;
	MEM_WB.rd = ID_EX.rd;
	MEM_WB.funct = ID_EX.funct;
	MEM_WB.opcode = EX_MEM.opcode;

		if(EX_MEM.opcode == 0x00){
			switch(ID_EX.funct){
				case 0x20: //ADD
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x21: //ADDU 
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x22: //SUB
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x23: //SUBU
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x24: //AND
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x25: //OR
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x26: //XOR
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x27: //NOR
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x08: //ADDI
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x09: //ADDIU
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x0A: //SLTI
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x0C: //ANDI
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x0D: //ORI
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				case 0x0E: //XORI
					MEM_WB.ALUOutput = EX_MEM.ALUOutput;
					break;
				default:
					printf("operation not implemented yet\n");
			}	
			return;
		}
		
		switch(EX_MEM.opcode){
			case 0x20: //LB
				data = mem_read_32(EX_MEM.ALUOutput);
				MEM_WB.LMD = ((data & 0x000000FF) & 0x80) > 0 ? (data | 0xFFFFFF00) : (data & 0x000000FF);
				print_instruction(ID_EX.PC);
				break;						
			case 0x21: //LH
				data = mem_read_32(EX_MEM.ALUOutput);
				MEM_WB.LMD = ((data & 0x0000FFFF) & 0x8000) > 0 ? (data | 0xFFFF0000) : (data & 0x0000FFFF);
				print_instruction(ID_EX.PC);
				break;
			case 0x23: //LW
				MEM_WB.ALUOutput = mem_read_32(EX_MEM.ALUOutput);
				print_instruction(ID_EX.PC);
				break;
			case 0x28: //SB
				data = mem_read_32(EX_MEM.ALUOutput);
				EX_MEM.B = (data & 0xFFFFFF00) | (EX_MEM.B & 0x000000FF);
				mem_write_32(EX_MEM.ALUOutput, MEM_WB.B);
				print_instruction(ID_EX.PC);				
				break;
			case 0x29: //SH
				data = mem_read_32(EX_MEM.ALUOutput);
				EX_MEM.B = (data & 0xFFFF0000) | (EX_MEM.B & 0x0000FFFF);
				mem_write_32(EX_MEM.ALUOutput, MEM_WB.B);
				print_instruction(ID_EX.PC);
				break;
			case 0x2B: //SW
				mem_write_32(EX_MEM.ALUOutput,MEM_WB.B);
				print_instruction(ID_EX.PC);
				break;
			default:
				// put more things here
				printf("Instruction at 0x%x is not implemented!\n", ID_EX.PC);
				break;

		}
	
	







}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{




	/*IMPLEMENT THIS*/
// 	In this stage, we have an ALU that operates on the operands that were read in the previous stage. We
// can perform one of three functions depending on the instruction type.

// i) Memory Reference (load/store):
// ALUOutput <= A + imm
// ALU adds two operands to form the effective address and stores the result into a register called
// ALUOutput.

// ii) Register-register Operation
// ALUOutput <= A op B
// ALU performs the operation specified by the instruction on the values stored in temporary registers A
// and B and places the result into ALUOutput.


// iii) Register-Immediate Operation
// ALUOutput <= A op imm
// ALU performs the operation specified by the instruction on the value stored in temporary register A and
// value in register imm and places the result into ALUOutput.



	uint32_t opcode, function, rs, rt, rd, sa, immediate;
	uint32_t target;
	uint64_t product, p1, p2;
	//uint32_t addr; unused variable
	uint32_t data;  
	int branch_jump = FALSE;
	opcode = ID_EX.opcode;
	function = ID_EX.funct;
	rs = ID_EX.A;
	rt =ID_EX.B;
	rd =ID_EX.rd;
	immediate = ID_EX.imm;
	target = ID_EX.tar;
	EX_MEM.opcode = opcode;
	EX_MEM.A = rs;
	EX_MEM.B = rt;
	EX_MEM.rd = rd;
	sa = ID_EX.shampt;

	// uint32_t nextPC = ID_EX.PC + 4; 

	if(opcode == 0x00){
		switch(function){
			case 0x00: //SLL  -- R type 
				EX_MEM.ALUOutput = rt << sa;
				print_instruction(ID_EX.PC);
				break;
			case 0x02: //SRL
				EX_MEM.ALUOutput = rt >> sa;
				print_instruction(ID_EX.PC);
				break;
			case 0x03: //SRA 
				if ((rt & 0x80000000) == 1)
				{
					EX_MEM.ALUOutput =  ~(~rt >> sa );
				}
				else{
					EX_MEM.ALUOutput = rt >> sa;
				}
				print_instruction(ID_EX.PC);
				break;
			case 0x08: //JR
				// nextPC = rs;
				// branch_jump = TRUE;
				// print_instruction(ID_EX.PC);
				break;
			case 0x09: //JALR
				// NEXT_STATE.REGS[rd] = ID_EX.PC + 4;
				// NEXT_STATE.PC = rs;
				// branch_jump = TRUE;
				// print_instruction(ID_EX.PC);
				break;
			case 0x0C: //SYSCALL
				if(CURRENT_STATE.REGS[2] == 0xa){
					RUN_FLAG = FALSE;
					print_instruction(ID_EX.PC);
				}
				break;
			case 0x10: //MFHI
				EX_MEM.ALUOutput = ID_EX.HI;
				print_instruction(ID_EX.PC);
				break;
			case 0x11: //MTHI  -???
				NEXT_STATE.HI = rs;
				print_instruction(ID_EX.PC);
				break;
			case 0x12: //MFLO - ???  
				NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
				print_instruction(ID_EX.PC);
				break;
			case 0x13: //MTLO - ??? 
				NEXT_STATE.LO = rs;
				print_instruction(ID_EX.PC);
				break;
			case 0x18: //MULT - ??? 
				if ((rs & 0x80000000) == 0x80000000){
					p1 = 0xFFFFFFFF00000000 | rs;
				}else{
					p1 = 0x00000000FFFFFFFF & rs;
				}
				if ((rt & 0x80000000) == 0x80000000){
					p2 = 0xFFFFFFFF00000000 | rt;
				}else{
					p2 = 0x00000000FFFFFFFF & rt;
				}
				product = p1 * p2;
				NEXT_STATE.LO = (product & 0X00000000FFFFFFFF);
				NEXT_STATE.HI = (product & 0XFFFFFFFF00000000)>>32;
				print_instruction(ID_EX.PC);
				break;
			case 0x19: //MULTU ???
				product = (uint64_t)rs * (uint64_t)rt;
				NEXT_STATE.LO = (product & 0X00000000FFFFFFFF);
				NEXT_STATE.HI = (product & 0XFFFFFFFF00000000)>>32;
				print_instruction(ID_EX.PC);
				break;
			case 0x1A: //DIV  -???
				if(rt != 0)
				{
					NEXT_STATE.LO = (int32_t)rs / (int32_t)rt;
					NEXT_STATE.HI = (int32_t)rs % (int32_t)rt;
				}
				print_instruction(ID_EX.PC);
				break;
			case 0x1B: //DIVU -????
				if(rt != 0)
				{
					NEXT_STATE.LO = rs / rt;
					NEXT_STATE.HI = rs % rt;
				}
				print_instruction(ID_EX.PC);
				break;
			case 0x20: //ADD
				EX_MEM.ALUOutput = rs + rt;
				print_instruction(ID_EX.PC);
				break;
			case 0x21: //ADDU 
				EX_MEM.ALUOutput = rt + rs;
				print_instruction(ID_EX.PC);
				break;
			case 0x22: //SUB
				EX_MEM.ALUOutput = rs - rt;
				print_instruction(ID_EX.PC);
				break;
			case 0x23: //SUBU
				EX_MEM.ALUOutput = rs - rt;
				print_instruction(ID_EX.PC);
				break;
			case 0x24: //AND
				EX_MEM.ALUOutput = rs & rt;
				print_instruction(ID_EX.PC);
				break;
			case 0x25: //OR
				EX_MEM.ALUOutput = rs | rt;
				print_instruction(ID_EX.PC);
				break;
			case 0x26: //XOR
				EX_MEM.ALUOutput = rs ^ rt;
				print_instruction(ID_EX.PC);
				break;
			case 0x27: //NOR
				EX_MEM.ALUOutput = ~(rs | rt);
				print_instruction(ID_EX.PC);
				break;
			case 0x2A: //SLT
				if(rs < rt){
					EX_MEM.ALUOutput = 0x1;
				}
				else{
					EX_MEM.ALUOutput = 0x0;
				}
				print_instruction(ID_EX.PC);
				break;
			default:
				printf("Instruction at 0x%x is not implemented!\n", ID_EX.PC);
				break;
		}
	}
	else{
		switch(opcode){
			case 0x01:
				if(rt == 0x00000){ //BLTZ
					// if((rs & 0x80000000) > 0){
					// 	NEXT_STATE.PC = ID_EX.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
					// 	branch_jump = TRUE;
					// }
					// print_instruction(ID_EX.PC);
				}
				else if(rt == 0x00001){ //BGEZ
					// if((rs & 0x80000000) == 0x0){
					// 	NEXT_STATE.PC = ID_EX.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
					// 	branch_jump = TRUE;
					// }
					// print_instruction(ID_EX.PC);
				}
				break;
			case 0x02: //J
				// NEXT_STATE.PC = (ID_EX.PC & 0xF0000000) | (target << 2);
				// branch_jump = TRUE;
				// print_instruction(ID_EX.PC);
				break;
			case 0x03: //JAL
				// NEXT_STATE.PC = (ID_EX.PC & 0xF0000000) | (target << 2);
				// NEXT_STATE.REGS[31] = ID_EX.PC + 4;
				// branch_jump = TRUE;
				// print_instruction(ID_EX.PC);
				break;
			case 0x04: //BEQ
				// if(rs == rt){
				// 	NEXT_STATE.PC = ID_EX.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
				// 	branch_jump = TRUE;
				// }
				// print_instruction(ID_EX.PC);
				break;
			case 0x05: //BNE
				// if(rs != rt){
				// 	NEXT_STATE.PC = ID_EX.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
				// 	branch_jump = TRUE;
				// }
				// print_instruction(ID_EX.PC);
				break;
			case 0x06: //BLEZ
				// if((rs & 0x80000000) > 0 || rs == 0){
				// 	NEXT_STATE.PC = ID_EX.PC +  ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
				// 	branch_jump = TRUE;
				// }
				// print_instruction(ID_EX.PC);
				break;
			case 0x07: //BGTZ
				// if((rs & 0x80000000) == 0x0){
				// 	NEXT_STATE.PC = ID_EX.PC +  ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
				// 	branch_jump = TRUE;
				// }
				// print_instruction(ID_EX.PC);
				break;
			case 0x08: //ADDI
				EX_MEM.ALUOutput = rs + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF));
				print_instruction(ID_EX.PC);
				break;
			case 0x09: //ADDIU
				EX_MEM.ALUOutput = rs + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF));
				print_instruction(ID_EX.PC);
				break;
			case 0x0A: //SLTI
				if ( (  (int32_t)rs - (int32_t)( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF))) < 0){
					EX_MEM.ALUOutput = 0x1;
				}else{
					EX_MEM.ALUOutput = 0x0;
				}
				print_instruction(ID_EX.PC);
				break;
			case 0x0C: //ANDI
				EX_MEM.ALUOutput = rs & (immediate & 0x0000FFFF);
				print_instruction(ID_EX.PC);
				break;
			case 0x0D: //ORI
				EX_MEM.ALUOutput = rs | (immediate & 0x0000FFFF);
				print_instruction(ID_EX.PC);
				break;
			case 0x0E: //XORI
				EX_MEM.ALUOutput = rs ^ (immediate & 0x0000FFFF);
				print_instruction(ID_EX.PC);
				break;
			case 0x0F: //LUI
				EX_MEM.ALUOutput = immediate << 16;
				print_instruction(ID_EX.PC);
				break;
			case 0x20: //LB
				data = rs + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF));
				EX_MEM.ALUOutput = data;
				print_instruction(ID_EX.PC);
				break;
			case 0x21: //LH
				EX_MEM.ALUOutput = rs + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF)) ;
				print_instruction(ID_EX.PC);
				break;
			case 0x23: //LW
				EX_MEM.ALUOutput = rs + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF)) ;
				print_instruction(ID_EX.PC);
				break;
			case 0x28: //SB
				EX_MEM.ALUOutput = rs + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF));
				// data = mem_read_32( addr);
				// data = (data & 0xFFFFFF00) | (rt & 0x000000FF);
				// mem_write_32(addr, data);
				print_instruction(ID_EX.PC);				
				break;
			case 0x29: //SH
				EX_MEM.ALUOutput = rs + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF));
				// data = mem_read_32( addr);
				// data = (data & 0xFFFF0000) | (rt & 0x0000FFFF);
				// mem_write_32(addr, data);
				print_instruction(ID_EX.PC);
				break;
			case 0x2B: //SW
				EX_MEM.ALUOutput = rs + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000) : (immediate & 0x0000FFFF));
				// mem_write_32(addr, rt);
				print_instruction(ID_EX.PC);
				break;
			default:
				// put more things here
				printf("Instruction at 0x%x is not implemented!\n", ID_EX.PC);
				break;
		}
	}
	
	if(!branch_jump){					
		NEXT_STATE.PC = ID_EX.PC;
	}

}


/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	/*IMPLEMENT THIS*/

// A <= REGS[rs]
// B <= REGS[rt]
// imm <= sign-extended immediate field of IR
// In this stage, the instruction is decoded (i.e., opcode and operands are extracted), and the content of the
// register file is read (rs and rt are the register specifiers that indicate which registers to read from). The
// values read from register file are placed into two temporary registers called A and B. The values stored 
// in A and B will be used in upcoming cycles by other stages (e.g., EX, or MEM). The lower 16 bits of
// the IR is sign-extended to 32-bit and stored in temporary register called imm. The value stored in imm
// register will be used in the next stage (i.e., EX).h



	uint32_t instruction = 0;
	instruction = IF_ID.IR;

	uint32_t opcode, function, rs, rt, rd, sa, immediate, target;
	
	
	
	
	instruction = mem_read_32(IF_ID.PC);
	
	opcode = (instruction & 0xFC000000) >> 26;
	function = instruction & 0x0000003F;
	rs = (instruction & 0x03E00000) >> 21;
	rt = (instruction & 0x001F0000) >> 16;
	rd = (instruction & 0x0000F800) >> 11;
	sa = (instruction & 0x000007C0) >> 6;
	immediate = instruction & 0x0000FFFF;
	target = instruction & 0x03FFFFFF;

	ID_EX.A = rs;
	ID_EX.B = rt;
	ID_EX.imm = (int) immediate;
	ID_EX.opcode = opcode;
	ID_EX.rd = rd;
	ID_EX.funct = function;
	ID_EX.shampt = sa;
	ID_EX.tar = target;
	
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
/*IMPLEMENT THIS*/
// IR <= Mem[PC]
// PC <= PC + 4
// The instruction is fetched from memory into the instruction register (IR) by using the current program
// counter (PC). The PC is then incremented by 4 to address the next instruction. IR is used to hold the
// instruction (that is 32-bit) that will be needed in subsequent cycle during the instruction decode stage.


	IF_ID.IR = mem_read_32(CURRENT_STATE.PC);
	IF_ID.PC = CURRENT_STATE.PC + 4;


}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	ID_EX.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	/*IMPLEMENT THIS*/
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
printf("Current PC: 0x%u \n",ID_EX.PC);
printf("IF/ID.IR 0x%u \n",IF_ID.IR );
printf("IF/ID.PC 0x%u \n",IF_ID.PC);
printf("ID/EX.IR 0x%u \n",ID_EX.IR);
printf("ID/EX.A 0x%u \n",ID_EX.A);
printf("ID/EX.B 0x%u \n",ID_EX.B);
printf("ID/EX.imm 0x%u \n",ID_EX.imm);
printf("EX/MEM.IR 0x%u \n",EX_MEM.IR);
printf("EX/MEM.A 0x%u \n",EX_MEM.A);
printf("EX/MEM.B 0x%u \n",EX_MEM.B);
printf("EX/MEM.ALU 0x%u \n",EX_MEM.ALUOutput);
printf("MEM/WB.IR 0x%u \n",MEM_WB.IR);
printf("MEM/WB.ALUOutput 0x%u \n",MEM_WB.ALUOutput);
printf("MEM/WB.LMD 0x%u \n",MEM_WB.LMD);

/*
Current PC: value
IF/ID.IR value instruction( e.g. add $1, $2, $3)
IF/ID.PC value //notice that it contains the next PC
ID/EX.IR value instruction
ID/EX.A value
ID/EX.B value
ID/EX.imm value
EX/MEM.IR value
EX/MEM.A value
EX/MEM.B value
EX/MEM.ALUOutput value
MEM/WB.IR value
MEM/WB.ALUOutput value
MEM/WB.LMD value
*/
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
