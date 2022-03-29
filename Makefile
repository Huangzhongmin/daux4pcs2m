CFLAGS += -O2
LDLIBS += -lm -lrt

#FLAGS = -I/usr/local/mdsplus/include -L/usr/local/mdsplus/lib ./include/librfm2g.a
#LDFLAG = -lMdsLib




all: apps


APPS := afhba-llcontrol-cpucopy 
	
	
apps: $(APPS)



afhba-llcontrol-cpucopy: DAQ-rfm-forpcs.c
#	$(CC) -o $@ $^ $(FLAGS) $(LDFLAG)
	cc -o afhba-llcontrol-cpucopy DAQ-rfm-forpcs.c -lpopt  -lpthread ./include/librfm2g.a
	
	
clean:
	rm -f $(APPS) afhba.0.log ftlach.log


	
