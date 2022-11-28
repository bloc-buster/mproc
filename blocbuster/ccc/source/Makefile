# Makefile for BlocBuster
# Sharlee Climer


CC	= g++
CFLAGS 	= -g
TARGET	= ccc
OBJS	= bloc.o

$(TARGET):	$(OBJS)
		$(CC) -o $(TARGET) $(OBJS)

bloc.o:		bloc.cpp bloc.h timer.h
		$(CC) $(CFLAGS) -c bloc.cpp

clean:
		/bin/rm -f *.o $(TARGET)
