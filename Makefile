CC = gcc

CFLAGS += -O2
LDLIBS += -lm -lrt -lstdc++

INCLUDE = -I/usr/local/mdsplus/include -I/usr/lib64/rfm2g
LD = -L/usr/local/mdsplus/lib -L/usr/lib64/rfm2g -L.
LIB = -lMdsLib -lrfm2g


all: daux 


daux: daux.c  
	$(CC) -Wall -g -o daux daux.c -lpopt -lpthread ${INCLUDE} ${LD} ${LIB} ${LDLIBS} -lHL2MCalSLib
	
	
clean:
	rm daux
	rm time.txt
	rm afhba.0.log
	
