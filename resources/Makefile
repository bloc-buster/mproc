CC = g++ 
CFLAGS = -g -lpthread -lm

all: mproc helper main
mproc: mproc.cpp
	$(CC) $^ -o $@ $(CFLAGS)
helper: helper.cpp
	$(CC) $^ -o $@ $(CFLAGS)
main: main.cpp
	$(CC) $^ -o $@ $(CFLAGS)
ccc: bloc.cpp
	$(CC) $^ -o $@ $(CFLAGS)
.PHONY: clean
clean:
	rm -rf mproc helper *.o *.dSYM *.key *.gml *.out

