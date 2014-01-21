/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x86 version 1.3.0
** DO NOT EDIT! The original file is in "..\brainfuck_jit.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

# 1 "..\brainfuck_jit.dasc"
#pragma once

#ifndef __BRAINFUCK_JIT
#define __BRAINFUCK_JIT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
	
//|.arch x86
//|.actionlist actions
static const unsigned char actions[121] = {
  85,83,86,87,137,229,129,252,236,239,139,93,20,137,252,239,129,252,239,239,
  49,210,255,128,3,235,255,128,43,235,255,129,195,239,255,129,252,235,239,255,
  15,182,3,136,4,23,66,129,252,250,239,15,140,244,247,198,4,23,0,137,60,36,
  232,243,49,210,248,1,255,137,84,36,252,252,232,243,136,3,139,84,36,252,252,
  255,252,233,245,250,15,249,255,250,15,249,128,59,0,15,133,245,255,198,4,23,
  0,137,60,36,232,243,49,192,137,252,236,95,94,91,93,195,255
};

# 12 "..\brainfuck_jit.dasc"
//|.section code
#define DASM_SECTION_CODE	0
#define DASM_MAXSECTION		1
# 13 "..\brainfuck_jit.dasc"
//|.globals GLOB_
enum {
  GLOB__MAX
};
# 14 "..\brainfuck_jit.dasc"

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
	//| push   ebp
	//| push   ebx
	//| push   esi
	//| push   edi
	//|
	//| // Create working area
	//| mov    ebp, esp
	//| sub    esp, OUT_BUFFSIZE+64
	//|
	//| // [ebp+20] = first argument, the memory buffer
	//| mov    ebx, dword [ebp+20]
	//|
	//| // Output buffering
	//| mov    edi, ebp
	//| sub    edi, OUT_BUFFSIZE+4
	//| xor    edx, edx
	dasm_put(Dst, 0, OUT_BUFFSIZE+64, OUT_BUFFSIZE+4);
# 46 "..\brainfuck_jit.dasc"
	
	char last = 0;
	int count = 0;
	
	for (int i = 0; i < len; i++) {
		char in = code[i];
		
		if (in != last) {
			switch (last) {
				case '+': 
					//| add byte [ebx], count
					dasm_put(Dst, 23, count);
# 57 "..\brainfuck_jit.dasc"
					break;
				case '-':
					//| sub byte [ebx], count
					dasm_put(Dst, 27, count);
# 60 "..\brainfuck_jit.dasc"
					break;
				case '>': 
					//| add ebx, count
					dasm_put(Dst, 31, count);
# 63 "..\brainfuck_jit.dasc"
					break;
				case '<': 
					//| sub ebx, count
					dasm_put(Dst, 35, count);
# 66 "..\brainfuck_jit.dasc"
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
				//| movzx eax, byte [ebx]
				//| mov   byte [edi+edx], al
				//| inc   edx
				//|
				//| // If buffer is not overflowing, do not print
				//| cmp   edx, OUT_BUFFSIZE
				//| jl    >1
				//|
				//| // Print out
				//| mov   byte [edi+edx], 0
				//| mov   [esp], edi
				//| call  &printf
				//| xor   edx, edx
				//|
				//| 1:
				dasm_put(Dst, 40, OUT_BUFFSIZE, (ptrdiff_t)(printf));
# 108 "..\brainfuck_jit.dasc"
				break; 

			case ',':
				//| // Need to store edx across call
				//| mov   [esp-4], edx
				//| call  &getchar
				//| mov   byte [ebx], al
				//| mov   edx, [esp-4]
				dasm_put(Dst, 69, (ptrdiff_t)(getchar));
# 116 "..\brainfuck_jit.dasc"
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
		        dasm_put(Dst, 84, (maxpc-2), (maxpc-1));
# 130 "..\brainfuck_jit.dasc"
			    break;
			    
			case ']':
				if (top == pcstack)
					return 0;
				
		        top--;
		        
		        //|.align 16
		        //| =>(*top-2):
		        //| cmp   byte [ebx], 0
		        //| jne   =>(*top-1)
		        dasm_put(Dst, 91, (*top-2), (*top-1));
# 142 "..\brainfuck_jit.dasc"
		        break;
		}
	}

	//| // Print remaining char in buffer
	//| mov     byte [edi+edx], 0
	//| mov     [esp], edi
	//| call    &printf
	//|
	//| // Function epilogue
	//| xor eax, eax
	//| mov esp, ebp
	//| pop edi
	//| pop esi
	//| pop ebx
	//| pop ebp
	//| ret
	dasm_put(Dst, 101, (ptrdiff_t)(printf));
# 159 "..\brainfuck_jit.dasc"

	
	BrainfuckBinary fptr = (BrainfuckBinary) jitcode(&state);
	return fptr;
}

#undef Dst
#endif

