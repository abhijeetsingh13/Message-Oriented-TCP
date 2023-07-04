LIBPATH=/usr/local/lib
INCPATH=/usr/local/include
library: mysocket.o
	@ echo
	@ echo "Creating library libmsocket.a"
	ar rcs libmsocket.a mysocket.o

install:
	@ echo
	@ echo "Installing library libmsocket.a"
	cp -p libmsocket.a $(LIBPATH)
	cp -p mysocket.h $(INCPATH)

mysocket.o: mysocket.c

test: server client

server: server.c
	gcc -Wall -o server server.c -lmsocket

client: client.c
	gcc -Wall -o client client.c -lmsocket

remove:
	rm -f server client mysocket.o

clean:
	rm -f  *.o libmsocket.a server client