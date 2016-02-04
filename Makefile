CC = gcc
CFLAGS = -std=gnu99 -Wall
CFLAGS += -O3
#CFLAGS += -march=native
#CFLAGS += -g
#CFLAGS += -pg
#CFLAGS += -fno-inline
#CFLAGS += -fprofile-arcs -ftest-coverage

OBJS = simulator.o utils.o main.o memory.o interrupt.o io.o dps.o gci.o monitor.o
SCI_SOCKET = /tmp/sci.sock

mist32_simulator: $(OBJS) $(FIFO)
	$(CC) $(CFLAGS) -lrt -lelf -lmsgpack -o $@ $(OBJS)

.c.o: common.h
	$(CC) $(CFLAGS) -c $<

dispatch.h: opsgen.py opcodes.py
	python opsgen.py dispatch.h

# FIXME
common.h: memory.h mmu.h vm.h dps.h sci.h utils.h debug.h registers.h
simulator.o: instructions.h insn_format.h dispatch.h fetch.h tlb.h

install: mist32_simulator
	cp mist32_simulator /usr/local/bin/

clean:
	rm -f *.o *.pyc mist32_simulator dispatch.h

listen-sci:
	@while true; do socat UNIX-LISTEN:$(SCI_SOCKET) STDIO; done

listen-sci-nc:
	@while true; do nc -l -U $(SCI_SOCKET); done
