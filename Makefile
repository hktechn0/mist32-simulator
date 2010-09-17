OBJS = simulator.o opcode_table.o instructions.o tools.o

simulator: $(OBJS)
	cc -o $@ $(OBJS)

.c.o:
	cc -c $<
