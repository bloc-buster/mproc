# Makefile for split
# Peter Holly
# June, 2011


CC	= g++
CFLAGS 	= -g
TARGET	= split 
OBJS	= split.o

$(TARGET):	$(OBJS)
		$(CC) -o $(TARGET) $(OBJS)

split.o:	split.cpp
		$(CC) $(CFLAGS) -c split.cpp

clean:
		/bin/rm -f *.o $(TARGET)
