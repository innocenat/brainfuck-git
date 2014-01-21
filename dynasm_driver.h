#pragma once

#ifndef __DYNASM_DRIVER
#define __DYNASM_DRIVER

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/mman.h>
#endif

#include "dynasm/dasm_proto.h"
#include "dynasm/dasm_x86.h"

void initjit(dasm_State **state, const void *actionlist, int label_count) {
	void* global_labels[label_count];

	dasm_init(state, 1);
	dasm_setupglobal(state, global_labels, label_count);
	dasm_setup(state, actionlist);
}

void* jitcode(dasm_State **state) {
	size_t size;
	int dasm_status = dasm_link(state, &size);
	assert(dasm_status == DASM_S_OK);

	// Allocate memory readable and writable so we can
	// write the encoded instructions there.
#ifdef _WIN32
	char* mem = (char*) VirtualAlloc(0, size + sizeof(size_t), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	assert(mem != 0);
#else
	char* mem = (char*) mmap(NULL, size + sizeof(size_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	assert(mem != MAP_FAILED);
#endif

	// Store length at the beginning of the region, so we
	// can free it without additional context.
	*(size_t*)mem = size;
	void *ret = mem + sizeof(size_t);

	dasm_encode(state, ret);
	dasm_free(state);

	// Adjust the memory permissions so it is executable
	// but no longer writable.
#ifdef _WIN32
	DWORD oldProt=0;
	BOOL st = VirtualProtect(mem, size, PAGE_EXECUTE_READ, &oldProt);
	assert(st);
#else
	int success = mprotect(mem, size, PROT_EXEC | PROT_READ);
	assert(success == 0);
#endif

#ifndef NDEBUG
	// Write generated machine code to a temporary file.
	// View with:
	//  objdump -D -b binary -mi386 -Mx86,intel jitcode > jitcode.asm
	FILE *f = fopen("jitcode", "wb");
	fwrite(ret, size, 1, f);
	fclose(f);
#endif

	return ret;
}

void free_jitcode(void *code) {
	void *mem = (char*)code - sizeof(size_t);
#ifdef _WIN32
	BOOL ret = VirtualFree(mem, 0, MEM_RELEASE);
	assert(ret);
#else
	int status = munmap(mem, *(size_t*)mem);
	assert(status == 0);
#endif
}

#endif

