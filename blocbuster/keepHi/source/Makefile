# Makefile for keeping highest weighted edges in gml file
# Sharlee Climer
# June, 2011


CC	= g++
CFLAGS 	= -g
TARGET	= keepHi
OBJS	= keepHighWt.o 

$(TARGET):	$(OBJS)
		$(CC) -o $(TARGET) $(OBJS)

keepHighWt.o:	keepHighWt.cpp keepHighWt.h timer.h
		$(CC) $(CFLAGS) -c keepHighWt.cpp


clean:
		/bin/rm -f *.o $(TARGET)
