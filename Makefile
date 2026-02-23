CC ?= tcc
CFLAGS = $(shell pkg-config --cflags libcurl libcjson)
LDFLAGS = $(shell pkg-config --libs libcurl libcjson)

all: piscord

piscord: example.o
	$(CC) -o piscord example.o $(LDFLAGS)

example.o: example.c piscord.h
	$(CC) -c example.c $(CFLAGS)

clean:
	rm -f piscord example.o tests/test_piscord

run: piscord
	./piscord

test: tests/test_piscord
	./tests/test_piscord

includes/tap.h:
	mkdir -p includes
	curl -sSL https://raw.githubusercontent.com/bernardo-bruning/tap-h/master/tap.h -o includes/tap.h

tests/test_piscord: tests/test_piscord.c piscord.h includes/tap.h
	$(CC) -o tests/test_piscord tests/test_piscord.c -I.
