CC = gcc
CFLAGS = -Wall
LIBS = -lws2_32

all: client router server

client: client.c
	$(CC) $(CFLAGS) client.c -o client $(LIBS)

router: router.c
	$(CC) $(CFLAGS) router.c -o router $(LIBS)

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(LIBS)

clean:
	del client.exe router.exe server.exe 2>nul || rm -f client router server
🔹 Compilation
make

🔹 Nettoyage
make clean
