#include "dynasm_driver.h"

#if defined(__x86_64__) || defined(_M_X64)
# ifdef _WIN64
#  include "brainfuck_jit_amd64_win.h"
# else
#  include "brainfuck_jit_amd64_linux.h"
# endif
#else
# include "brainfuck_jit.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stdout, "Usage: %s [input]", argv[0]);
		return 0;
	}
	
	FILE* fin = fopen(argv[1], "rb");
	
	if (!fin) {
		fprintf(stderr, "Error: cannot open file %s", argv[1]);
		return 1;
	}
	
	// Find file size
	struct stat st;
	size_t fsize;

    if (stat(argv[1], &st) == 0)
        fsize = st.st_size;
    else {
		fprintf(stderr, "Error: cannot read file size.");
		return -1;
    }
	
	// Allocate buffer
	char* buff = (char*) malloc(fsize+1);
	if (!buff) {
		fprintf(stderr, "Error: cannot allocate memory.");
		return 2;
	}
	
	// Read file
	size_t read_count = fread(buff, 1, fsize, fin);
	if (read_count != fsize) {
		fprintf(stderr, "Error: file read error: size: %d; read %d.", fsize, read_count);
		return 3;
	}

	// Close file
	fclose(fin);
	
	// Create binary
	BrainfuckBinary fptr = brainfuck_create_binary(buff, fsize, 256, 256);
	free(buff);
	if (!fptr) {
		fprintf(stderr, "Error: cannot create brainfuck binary");
		return 4;
	}
	
	// Allocate brainfuck memory
	char* mem = (char*) calloc(30000, 1);
	if (!mem) {
		free_jitcode((void*)fptr);
		fprintf(stderr, "Error: cannot allocate brainfuck memory");
		return 5;
	}
	
	// Execute
	int ret = fptr(mem);
	
	free(mem);
	free_jitcode((void*)fptr);
	
	if (ret) {
		fprintf(stderr, "Error: execution error");
		return 6;
	}
	
	return 0;
}


