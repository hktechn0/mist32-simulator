CC = cc
CFLAGS = -std=gnu99 -Wall -O3
#CFLAGS += -g
#CFLAGS += -pg
#CFLAGS += -fno-inline

OBJS = simulator.o utils.o main.o memory.o interrupt.o io.o dps.o gci.o monitor.o
SCI_SOCKET = sci.sock

mist32_simulator: $(OBJS) $(FIFO)
	$(CC) $(CFLAGS) -lrt -lelf -lmsgpack -o $@ $(OBJS)

.c.o: common.h
	$(CC) $(CFLAGS) -c $<

dispatch.h: opsgen.py opcodes.py
	python opsgen.py dispatch.h

common.h: memory.h instruction_format.h io.h

simulator.o: instructions.h dispatch.h
io.o monitor.o: monitor.h
interrupt.o: interrupt.h

install: mist32_simulator
	cp mist32_simulator /usr/local/bin/

clean:
	rm -f *.o *.pyc mist32_simulator dispatch.h

listen-sci:
	@while true; do socat UNIX-LISTEN:$(SCI_SOCKET) STDIO; done

listen-sci-nc:
	@while true; do nc -l -U $(SCI_SOCKET); done
