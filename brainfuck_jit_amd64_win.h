/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "brainfuck_jit_amd64_win.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

# 1 "brainfuck_jit_amd64_win.dasc"
#pragma once

#ifndef __BRAINFUCK_JIT
#define __BRAINFUCK_JIT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
	
//|.arch x64
//|.actionlist actions
static const unsigned char actions[163] = {
  85,65,87,65,86,65,85,65,84,72,137,229,72,129,252,236,239,73,137,207,72,49,
  201,73,137,252,238,73,129,252,238,239,77,49,228,255,65,128,7,235,255,65,128,
  47,235,255,73,129,199,239,255,73,129,252,239,239,255,73,15,182,7,67,136,4,
  38,73,252,255,196,73,129,252,252,239,15,140,244,247,67,198,4,38,0,76,137,
  252,241,77,49,228,72,184,237,237,252,255,208,248,1,255,72,184,237,237,252,
  255,208,65,136,7,255,252,233,245,250,15,249,255,250,15,249,65,128,63,0,15,
  133,245,255,67,198,4,38,0,76,137,252,241,72,184,237,237,252,255,208,72,49,
  192,72,137,252,236,65,92,65,93,65,94,65,95,93,195,255
};

# 12 "brainfuck_jit_amd64_win.dasc"
//|.section code
#define DASM_SECTION_CODE	0
#define DASM_MAXSECTION		1
# 13 "brainfuck_jit_amd64_win.dasc"
//|.globals GLOB_
enum {
  GLOB__MAX
};
# 14 "brainfuck_jit_amd64_win.dasc"
//|.define PRM, rcx

#define Dst &state

typedef int (__cdecl *BrainfuckBinary)(uint8_t* memory);

BrainfuckBinary brainfuck_create_binary(char* code, int len, int MAX_NESTING, int OUT_BUFFSIZE) {
	// Initialize GIT
	dasm_State *state;
	initjit(&state, actions, GLOB__MAX);
	
	// Dynamic label
	uint32_t maxpc = 0;
	int pcstack[MAX_NESTING];
	int *top = pcstack, *limit = pcstack + MAX_NESTING;
	
	//| // Store register
	//| push   rbp
	//| push   r15
	//| push   r14
	//| push   r13
	//| push   r12
	//|
	//| // Create working area
	//| mov    rbp, rsp
	//| sub    rsp, OUT_BUFFSIZE+64 // and shadow space is already reserved
	//|
	//| // PARA = first argument, the memory buffer
	//| mov    r15, PRM
	//| xor    PRM, PRM
	//|
	//| // Output buffering
	//| mov    r14, rbp
	//| sub    r14, OUT_BUFFSIZE+4
	//| xor    r12, r12
	dasm_put(Dst, 0, OUT_BUFFSIZE+64, OUT_BUFFSIZE+4);
# 49 "brainfuck_jit_amd64_win.dasc"
	
	char last = 0;
	int count = 0;
	
	for (int i = 0; i < len; i++) {
		char in = code[i];
		
		if (in != last) {
			switch (last) {
				case '+': 
					//| add byte [r15], count
					dasm_put(Dst, 36, count);
# 60 "brainfuck_jit_amd64_win.dasc"
					break;
				case '-':
					//| sub byte [r15], count
					dasm_put(Dst, 41, count);
# 63 "brainfuck_jit_amd64_win.dasc"
					break;
				case '>': 
					//| add r15, count
					dasm_put(Dst, 46, count);
# 66 "brainfuck_jit_amd64_win.dasc"
					break;
				case '<': 
					//| sub r15, count
					dasm_put(Dst, 51, count);
# 69 "brainfuck_jit_amd64_win.dasc"
					break;
			}
			
			count = 0;
		}
		
		last = in;
		
		switch (in) {
			case '+':
				count++;
				break;

			case '-':
				count++;
				break;

			case '>':
				count++;
				break;

			case '<':
				count++;
				break;

			case '.':
				//| // Store current char in buffer
				//| movzx rax, byte [r15]
				//| mov   byte [r14+r12], al
				//| inc   r12
				//|
				//| // If buffer is not overflowing, do not print
				//| cmp   r12, OUT_BUFFSIZE
				//| jl    >1
				//|
				//| // Print out
				//| mov   byte [r14+r12], 0
				//| mov   PRM, r14
				//| xor   r12, r12
				//| mov64 rax, (uintptr_t) printf
				//| call  rax
				//|
				//| 1:
				dasm_put(Dst, 57, OUT_BUFFSIZE, (unsigned int)((uintptr_t) printf), (unsigned int)(((uintptr_t) printf)>>32));
# 112 "brainfuck_jit_amd64_win.dasc"
				break; 

			case ',':
				//| mov64 rax, (uintptr_t) getchar
				//| call  rax
				//| mov   byte [r15], al
				dasm_put(Dst, 100, (unsigned int)((uintptr_t) getchar), (unsigned int)(((uintptr_t) getchar)>>32));
# 118 "brainfuck_jit_amd64_win.dasc"
				break;

			case '[':
				if (top == limit)
					return 0;
				
		        maxpc += 2;
		        *top++ = maxpc;
		        dasm_growpc(&state, maxpc);
		        
		        //| jmp   =>(maxpc-2)
		        //|
		        //|.align 16
		        //| =>(maxpc-1):
		        dasm_put(Dst, 111, (maxpc-2), (maxpc-1));
# 132 "brainfuck_jit_amd64_win.dasc"
			    break;
			    
			case ']':
				if (top == pcstack)
					return 0;
				
		        top--;
		        
		        //|.align 16
		        //| =>(*top-2):
		        //| cmp   byte [r15], 0
		        //| jne   =>(*top-1)
		        dasm_put(Dst, 118, (*top-2), (*top-1));
# 144 "brainfuck_jit_amd64_win.dasc"
		        break;
		}
	}

	//| // Print remaining char in buffer
	//| mov   byte [r14+r12], 0
	//| mov   PRM, r14
	//| mov64 rax, (uintptr_t) printf
	//| call  rax
	//|
	//| // Function epilogue
	//| xor rax, rax
	//| mov rsp, rbp
	//| pop r12
	//| pop r13
	//| pop r14
	//| pop r15
	//| pop rbp
	//| ret
	dasm_put(Dst, 129, (unsigned int)((uintptr_t) printf), (unsigned int)(((uintptr_t) printf)>>32));
# 163 "brainfuck_jit_amd64_win.dasc"

	
	BrainfuckBinary fptr = (BrainfuckBinary) jitcode(&state);
	return fptr;
}

#undef Dst
#endif

