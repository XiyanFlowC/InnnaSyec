codec.o: codec.c encode.inc
	cc codec.c -c -o codec.o

disas.o: disas.c
	cc disas.c -c -o disas.o

all: disas.o codec.o


clean:
	rm -f *.o