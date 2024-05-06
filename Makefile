CC = gcc
CFLAGS = --std=c99 -Wall -Werror -pedantic 
LIBS = -lncurses
BUILD_DIR = build

SIM: $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(BUILD_DIR)/main.o: main.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir $@

.PHONY: clean
clean:
	rm -f build/*
	rm -f SIM
