

OBJECTS = cpqarrayd.o discover.o status.o sendtrap.o
HFILES = cpqarrayd.h discover.h status.h sendtrap.h

CFLAGS=-I/usr/src/linux/drivers/block
LIBS=-lsnmp

cpqarrayd: $(OBJECTS) 
	$(CC) -o cpqarrayd $(OBJECTS) $(LIBS)
