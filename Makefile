CC     = gcc
CFLAGS = -O2 -Wall
LDLIBS = -lm

tracer: tracer.c
	$(CC) $(CFLAGS) tracer.c -o tracer $(LDLIBS)

run: tracer
	./tracer

clean:
	rm -f tracer

.PHONY: run clean
