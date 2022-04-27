CC = gcc

CFLAGS += -O2
LDLIBS += -lm -lrt

INCLUDE = -I/usr/local/mdsplus/include -I/usr/lib64/rfm2g
LD = -L/usr/local/mdsplus/lib -L/usr/lib64/rfm2g
LIB = -lMdsLib -lrfm2g


all: daux 


daux: daux.c
	$(CC) -o daux daux.c -lpopt  -lpthread ${INCLUDE} ${LD} ${LIB}
	
	
clean:
	rm daux 

	
