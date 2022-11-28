# Makefile for randomize
# Sharlee Climer
# January, 2011


CC	= g++
CFLAGS 	= -g
TARGET	= perm
OBJS	= randomize.o

$(TARGET):	$(OBJS)
		$(CC) -o $(TARGET) $(OBJS)

randomize.o:		randomize.cpp randomize.h timer.h
		$(CC) $(CFLAGS) -c randomize.cpp

clean:
		/bin/rm -f *.o $(TARGET)
