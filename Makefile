CFLAGS = $(shell pkg-config --cflags libcurl libcjson)
LDFLAGS = $(shell pkg-config --libs libcurl libcjson)

all: piscord

piscord: example.o
	gcc -o piscord example.o $(LDFLAGS)

example.o: example.c piscord.h
	gcc -c example.c $(CFLAGS)

clean:
	rm -f piscord example.o

run: piscord
	./piscord
