CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g

SRCS = src/main.c src/hashtable.c
OBJS = $(SRCS:.c=.o)
TARGET = db

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: src/hashtable.o
	$(CC) $(CFLAGS) -o unit_test tests/unit_test.c src/hashtable.o
	./unit_test

clean:
	rm -f $(OBJS) $(TARGET) unit_test wal.log test_wal_esc.log

.PHONY: all clean test
