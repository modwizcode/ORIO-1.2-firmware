CFLAGS+=-Wall -Werror

OBJ=\
	dump.o crcccitt.o

PROGS=dump

all: $(PROGS)

dump: $(OBJ)
clean:
	rm -f $(OBJ) $(PROGS)

world: clean all
