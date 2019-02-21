CC=cc
CFLAGS=-Wall -O3
OBJ = zcat.o
LDFLAGS=-lz
DEPS=

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

zcat: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	-rm -f $(OBJ)
	-rm -f zcat

