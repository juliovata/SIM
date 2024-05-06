/*
  sim.c
  Julio Vata - jvata@umich.edu - 2024
*/

#include <ncurses.h>
#include <stdbool.h>

#include "version.h"
#include "config.h"

int main() {
  // start ncurses
  initscr();

  // initialize buffer
  printw("SIM v%d.%d.%d\n", MAJOR, MINOR, PATCH);

  // start program loop
  bool running = true;
  do {
    // draw buffer
    refresh();
    napms(REFRESH_RATE_MS);

    // get user input
    char cur_input = getchar();

    // handle user input
    running = (cur_input != 'q');
  } while(running);

  // end ncurses
  endwin();

  return 0;
}