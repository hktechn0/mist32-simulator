CC = gcc
CFLAGS = -std=gnu99 -g -Wall -O2
OBJS = simulator.o opstable.o instructions.o utils.o main.o memory.o interrupt.o io.o dps.o gci.o monitor.o
FIFO = sci_txd sci_rxd gci_display_char

mist32_simulator: $(OBJS) $(FIFO)
	$(CC) $(CFLAGS) -lrt -lelf -lmsgpack -o $@ $(OBJS)

.c.o: common.h
	$(CC) $(CFLAGS) -c $<

common.h: memory.h instruction_format.h io.h

instructions.o opstable.o: instructions.h
io.o monitor.o: monitor.h
interrupt.o: interrupt.h

$(FIFO):
	mkfifo $@

clean:
	rm *.o mist32_simulator
