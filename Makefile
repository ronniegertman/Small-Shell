CC = gcc
CFLAGS = -std=c99 -Wall -Werror -pedantic-errors -pthread -DNDEBUG
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
TARGET = smash

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJS)
