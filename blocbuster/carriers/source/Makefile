# Makefile for determining carriers of clusters of alleles
# Sharlee Climer
# June, 2009


CC	= g++
CFLAGS 	= -g
TARGET	= carriers
OBJS	= carriers.o

$(TARGET):	$(OBJS)
		$(CC) -o $(TARGET) $(OBJS)

carriers.o:	carriers.cpp carriers.h timer.h
		$(CC) $(CFLAGS) -c carriers.cpp

clean:
		/bin/rm -f *.o $(TARGET)
