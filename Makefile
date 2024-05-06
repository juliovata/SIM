CC = gcc
CFLAGS = --std=c99 -Wall -Werror -pedantic 
LIBS = -lncurses
BUILD_DIR = build

sim: $(BUILD_DIR)/sim.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(BUILD_DIR)/sim.o: sim.c version.h config.h | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir $@

.PHONY: clean
clean:
	rm -f build/*
	rm -f sim
