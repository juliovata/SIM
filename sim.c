/*
  sim.c
  Julio Vata - jvata@umich.edu - 2024
*/

#include "sim.h"

#include <ncurses.h>
#include <stdbool.h>
#include <string.h>

#include "config.h"
#include "version.h"

int main() {
  // variables
  char cmd_buf[CMD_BUF_SIZE];
  char *cmd_write_ptr;
  bool running;
  bool mode;
  bool cmd_not_found;

  // start ncurses
  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();

  curs_set(0);
  ESCDELAY = 0;
#ifndef DEBUG
  nodelay(stdscr, TRUE);
#endif

  // initialize buffer
  printw("SIM v%d.%d.%d\n", MAJOR, MINOR, PATCH);
  printw("Julio Vata - jvata@umich.edu - 2024");

  // program loop
  running = true;
  mode = MODE_NORMAL;
  cmd_not_found = false;
  while (running) {
    // get user input
    int cur_input = getch();

    // handle user input
    if (cur_input != ERR) {
      if (mode == MODE_NORMAL) {
        switch (cur_input) {
          case 'q':
            running = false;
            break;
          case ':':
            // switch to command line mode
            mode = MODE_CMD;
            cmd_write_ptr = cmd_buf;
            cmd_not_found = false;
            memset(cmd_buf, '\0', CMD_BUF_SIZE);
            curs_set(1);
            break;
          default:
              // do nothing
              ;
        }
      } else if (mode == MODE_CMD) {
        if (cur_input == 27) {
          curs_set(0);
          mode = MODE_NORMAL;
        } else if (cur_input == '\n') {
          // execute command, exit command line mode
          curs_set(0);
          mode = MODE_NORMAL;

          if (cmd_buf != cmd_write_ptr) {
            if (strcmp(cmd_buf, "q") == 0) {
              running = false;
            } else {
              cmd_not_found = true;
            }
          }
        } else if (cur_input == KEY_BACKSPACE) {
          if (cmd_write_ptr != cmd_buf) {
            // remove character from buffer
            --cmd_write_ptr;
            *cmd_write_ptr = '\0';
          }
        } else if (cmd_write_ptr < (cmd_buf + CMD_BUF_SIZE - 1)) {
          // add input to buffer if there is space
          *cmd_write_ptr = (char)cur_input;
          ++cmd_write_ptr;
        }
      }
    }

    // update buffer
    int row = CMD_ROW_START;
    int col = CMD_COL_START;
    move(row, col);
    clrtoeol();
    if (cmd_not_found) {
      printw("%s", "COMMAND NOT FOUND");
    } else if (mode == MODE_CMD) {
      // print current command line
      addch(':');
      move(row, ++col);
      printw("%s", cmd_buf);
    }

    // draw buffer
    refresh();
    napms(REFRESH_RATE_MS);
  }

  // end ncurses
  endwin();

  return 0;
}