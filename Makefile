# Makefile for rmdss utility

CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = rmds
SRC = rmds.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

