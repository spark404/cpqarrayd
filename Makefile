

OBJECTS = cpqarrayd.o discover.o status.o sendtrap.o
HFILES = cpqarrayd.h discover.h status.h sendtrap.h

CFLAGS=-I/usr/src/linux/drivers/block -g
LIBS=-lsnmp

cpqarrayd: $(OBJECTS) 
	$(CC) -g -o cpqarrayd $(OBJECTS) $(LIBS)
