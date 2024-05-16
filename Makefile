CC = gcc
CFLAGS = --std=c99 -Wall -Wextra -Wconversion -pedantic
LIBS = -lncurses

BUILD_DIR = build

# Debug
debug: CFLAGS += -DDEBUG -O0 -g 
debug: $(BUILD_DIR)/debug/sim.o $(BUILD_DIR)/debug/process_command.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o sim_dbg

$(BUILD_DIR)/debug/sim.o: sim.c sim.h version.h process_command.h | $(BUILD_DIR)/debug
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug/process_command.o: process_command.c process_command.h | $(BUILD_DIR)/debug
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug:
	mkdir $@	

all: debug

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/debug/* sim_dbg