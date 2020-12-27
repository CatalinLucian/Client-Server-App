CC=gcc -Wall -Wextra

all: build

build: server subscriber

server:queue.o utils.o map.o server.o
				$(CC) $^ -o $@ -lm

server.o: server.c
					$(CC) $^ -c -lm -ggdb3 -g

subscriber: utils.o map.o queue.o subscriber.o
						$(CC) $^ -o $@ -lm

utils.o: utils.c
				 $(CC) $^ -c -lm

queue.o: queue.c
				 $(CC) $^ -c -lm

map.o: map.c
			 $(CC) $^ -c -lm

subscriber.o: subscriber.c
						 	$(CC) $^ -c -lm

clean:
				rm *.o server subscriber
