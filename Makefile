CC = gcc
CFLAGS = --std=c99 -Wconversion -Wall -Wextra -pedantic 
LIBS = -lncurses
BUILD_DIR = build

sim: $(BUILD_DIR)/sim.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o sim

debug: CFLAGS += -DDEBUG -g3
debug: $(BUILD_DIR)/sim.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o sim_dbg 

release: CFLAGS += -O3
release: sim

all: release sim_dbg

$(BUILD_DIR)/sim.o: sim.c sim.h config.h version.h | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir $@

.PHONY: clean
clean:
	rm -f build/* sim sim_dbg