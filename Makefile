CFLAGS = -g -Wall
OBJS = simulator.o opcode_table.o instructions.o tools.o

simulator: $(OBJS)
	cc $(CFLAGS) -o $@ $(OBJS)

.c.o:
	cc $(CFLAGS) -c $<

simulator.o: common.h
instructions.o: instructions.h

clean:
	rm *.o