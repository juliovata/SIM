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

  printw("SIM v%d.%d.%d\n", MAJOR, MINOR, PATCH);
  printw("Julio Vata - jvata@umich.edu - 2024\n");

  // program loop
  char cmd_buf[CMD_BUF_SIZE];
  char *cmd_write_ptr;
  bool running = true;
  bool mode = MODE_NORMAL;
  bool cmd_not_found = false;
  bool start_msg = true;
  int prev_width;
  int prev_height;

  getmaxyx(stdscr, prev_height, prev_width);

  while (running) {
    // get current window size
    int cur_width;
    int cur_height;
    getmaxyx(stdscr, cur_height, cur_width);

    // resize occured so clear everything on the screen
    // its ok to clear screen on first loop
    if ((cur_width != prev_width) || (cur_height != prev_height)) {
      start_msg = false;
      clear();
      refresh();
    }

    prev_width = cur_width;
    prev_height = cur_height;

    // handle received serial data
    refresh();

    // get user input
    int cur_input = getch();

    // handle user input
    // do not make any user input edits to the screen if a key is not pressed!
    if (cur_input != ERR) {
      if (mode == MODE_NORMAL) {
        switch (cur_input) {
          case ':':
            // switch to command line mode
            mode = MODE_CMD;
            cmd_not_found = false;

            cmd_write_ptr = cmd_buf;
            memset(cmd_buf, '\0', CMD_BUF_SIZE);

            curs_set(1);
            break;
          case 27:
            // clear error message
            cmd_not_found = false;
          default:
              // do nothing
              ;
        }
      } else if (mode == MODE_CMD) {
        if (cur_input == 27) {  // escape key
          mode = MODE_NORMAL;
          curs_set(0);
        }
#ifdef KEY_RESIZE
        else if (cur_input == KEY_RESIZE) {
          // do nothing
        }
#endif
        else if (cur_input == '\n') {  // enter key
          // execute command, exit command line mode
          mode = MODE_NORMAL;
          curs_set(0);

          if (cmd_buf != cmd_write_ptr) {  // if buffer is not empty
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
          // TODO(juliovata):   what to do about this conversion here?
          //                    only add ascii characters to buffer?
          *cmd_write_ptr = cur_input;
          ++cmd_write_ptr;
        }
      }
    }

    // update buffer
    int row = cur_height - 1;
    int col = 0;
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
