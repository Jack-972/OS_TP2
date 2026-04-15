CC = gcc
CFLAGS = -Wall -Wextra -Werror -D_REENTRANT
LDFLAGS = -lreadline -pthread

default: biceps

biceps: biceps.o creme.o gescom.o
	$(CC) $(CFLAGS) -o biceps biceps.o creme.o gescom.o $(LDFLAGS)

memory-leak: CFLAGS += -g -O0
memory-leak: biceps.o creme.o gescom.o
	$(CC) $(CFLAGS) -o biceps-memory-leaks biceps.o creme.o gescom.o $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o biceps biceps-memory-leaks
