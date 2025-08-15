CC=gcc
LIBFLAGS=
CFLAGS= -O2
DEBUGFLAGS= -Wall -g

SIMULATOR_FILES= Simulator/simulator.c Simulator/preCache.c
COMPILER_FILES= Compiler/compiler.c Compiler/scanner.c Compiler/stringTree.c Compiler/parser.c Compiler/translate.c
LIB_FILES= Lib/lib.c

sim:
	@if [ -f "sim" ]; then \
		rm "sim"; \
	fi
	$(CC) $(LIBFLAGS) $(SIMULATOR_FILES) $(LIB_FILES) $(CFLAGS) -o sim

simD:
	@if [ -f "simD" ]; then \
		rm "simD"; \
	fi
	$(CC) $(LIBFLAGS) $(SIMULATOR_FILES) $(LIB_FILES) $(DEBUGFLAGS) -o simD

comp:
	@if [ -f "comp" ]; then \
		rm "comp"; \
	fi
	$(CC) $(COMPILER_FILES) $(LIB_FILES) $(CFLAGS) -o comp

compD:
	@if [ -f "compD" ]; then \
		rm "compD"; \
	fi
	$(CC) $(COMPILER_FILES) $(LIB_FILES) $(DEBUGFLAGS) -o compD