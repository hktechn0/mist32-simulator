CC = gcc
CFLAGS = -std=gnu99 -g -Wall -O0
OBJS = simulator.o opstable.o instructions.o utils.o main.o memory.o io.o gci_device.o
FIFO = sci_txd sci_rxd gci_display_char

mist32_simulator: $(OBJS) $(FIFO)
	$(CC) $(CFLAGS) -lelf -o $@ $(OBJS)

.c.o: common.h
	$(CC) $(CFLAGS) -c $<

instructions.o: instructions.h
opstable.o: instructions.h
gci_device.o: gci_device.h
io.o: gci_device.h

common.h: memory.h instruction_format.h io.h

$(FIFO):
	mkfifo $@

clean:
	rm *.o mist32_simulator
