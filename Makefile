all: piscord

piscord: example.o
	gcc -o piscord example.o

piscord.o: example.c piscord.h
	gcc -c example.c

clean:
	rm -f piscord example.o

run: piscord
	./piscord
