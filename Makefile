CC = g++ 
CFLAGS = -g -lpthread -lm

all: mproc helper batch
mproc: mproc.cpp
	$(CC) $^ -o $@ $(CFLAGS)
helper: helper.cpp
	$(CC) $^ -o $@ $(CFLAGS)
batch: batch.cpp
	$(CC) $^ -o $@ $(CFLAGS)
.PHONY: clean
clean:
	rm -rf mproc helper *.o *.dSYM *.key *.gml *.out temp_output_files

