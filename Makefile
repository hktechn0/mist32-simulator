CC= gcc
CFLAGS = -g -Wall -O2
OBJS = simulator.o opstable.o instructions.o utils.o main.o memory.o

mist32_simulator: $(OBJS)
	$(CC) $(CFLAGS) -lelf -o $@ $(OBJS)

.c.o: common.h
	$(CC) $(CFLAGS) -c $<

instructions.o: instructions.h
opstable.o: instructions.h

common.h: memory.h instruction_format.h

clean:
	rm *.o mist32_simulator

