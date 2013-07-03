CC = gcc
CFLAGS = -std=gnu99 -g -Wall -O0
OBJS = simulator.o opstable.o instructions.o utils.o main.o memory.o io.o
FIFO = sci_txd sci_rxd

mist32_simulator: $(OBJS) $(FIFO)
	$(CC) $(CFLAGS) -lelf -o $@ $(OBJS)

.c.o: common.h
	$(CC) $(CFLAGS) -c $<

instructions.o: instructions.h
opstable.o: instructions.h

common.h: memory.h instruction_format.h

sci_txd sci_rxd:
	mkfifo $@

clean:
	rm *.o mist32_simulator

