TARGET = bin-edit
SRC = main.c
CC = gcc
CFLAGS = -Wall -O2
LIBS = -lncurses

.PHONY: all build run clean

all: build run

build:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

run: $(TARGET)
	./$(TARGET) test.bin

clean:
	rm -f $(TARGET)
