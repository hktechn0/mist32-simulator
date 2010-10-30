CFLAGS = -g -Wall
OBJS = simulator.o opstable.o instructions.o utils.o

simulator: $(OBJS)
	cc $(CFLAGS) -o $@ $(OBJS)

.c.o:
	cc $(CFLAGS) -c $<

instructions.o: instructions.h

clean:
	rm *.o
