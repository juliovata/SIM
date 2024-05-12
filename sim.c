/*
  sim.c
  Julio Vata - jvata@umich.edu - 2024

  TODO:   - In Command_line mode, update file line on input
          - update status bar on command_line mode exit
          - decouple printing serial data to screen from updating final line
          - Implement status bar in normal mode in same line as CMD
          - implement set_port, set_baud, set_format, open, close
          - Handle terminal resize with interrupt
          - Deal with int to char conversion
          - CMD line should expand in height when command text passes edge
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
  nodelay(stdscr, TRUE);
  curs_set(0);
  ESCDELAY = 0;  // program treats escape key like any other key

  printw("SIM v%d.%d.%d\n", MAJOR, MINOR, PATCH);
  printw("Julio Vata - jvata@umich.edu - 2024\n");

  // program loop
  program_mode mode = Normal;

  char cmd_buf[CMD_BUF_SIZE] = {'\0'};
  char *cmd_write_ptr = cmd_buf;

  bool cmd_error = false;
  char cmd_error_msg[ERROR_MSG_BUF_SIZE] = {'\0'};

  int prev_width = -1;
  int prev_height = -1;

  bool is_open = false;
  char port_name[PORT_NAME_BUF_SIZE] = "NO_PORT";
  unsigned int baud = 0;
  unsigned int data_bits = 0;
  unsigned int start_bits = 0;
  bool parity_bit = 0;
  bool recording_to_file = false;

  // initialize previous screen dimensions
  getmaxyx(stdscr, prev_height, prev_width);

  // program loop
  while (true) {
    // get current window size
    // detect if window size has changed
    int cur_width;
    int cur_height;
    getmaxyx(stdscr, cur_height, cur_width);

    bool win_resized = (cur_width != prev_width) || (cur_height != prev_height);

    prev_width = cur_width;
    prev_height = cur_height;

    // get user input
    int cur_input = getch();

    // handle user input
    if (cur_input != ERR) {
      if (mode == Normal) {
        switch (cur_input) {
          case ':':
            // switch to command line mode
            mode = Command_line;

            // reset command buffer
            cmd_write_ptr = cmd_buf;
            memset(cmd_buf, '\0', CMD_BUF_SIZE);

            // reset error handling
            cmd_error = false;
            memset(cmd_error_msg, '\0', ERROR_MSG_BUF_SIZE);

            // enable cursor
            curs_set(1);
            break;
          case ESCAPE_KEY:
            // clear error message
            cmd_error = false;
            memset(cmd_error_msg, '\0', ERROR_MSG_BUF_SIZE);
            break;
          default:
              // do nothing
              ;
        }
      } else if (mode == Command_line) {
        if (cur_input == ESCAPE_KEY) {
          // leave CMD mode and disable cursor
          mode = Normal;
          curs_set(0);
        }
#ifdef KEY_RESIZE
        // this bit of code here basically disables KEY_RESIZE which based on
        // my understanding is not particularly portable
        else if (cur_input == KEY_RESIZE) {
          // do nothing
        }
#endif
        else if (cur_input == '\n') {  // enter key
          // execute command, exit command line mode
          mode = Normal;
          curs_set(0);

          if (cmd_buf != cmd_write_ptr) {
            // if buffer is not empty try to execute whatever text is in the
            // buffer
            if (strcmp(cmd_buf, "q") == 0) {
              // quit program
              break;
            } else {
              // command not found, trigger error message
              cmd_error = true;
              strcpy(cmd_error_msg, "COMMAND NOT FOUND");
            }
          }
        } else if (cur_input == KEY_BACKSPACE) {
          // either remove character from buffer or if buffer is empty exit
          // cmd mode
          if (cmd_write_ptr != cmd_buf) {
            // remove character from buffer
            --cmd_write_ptr;
            *cmd_write_ptr = '\0';
          } else if (cmd_write_ptr == cmd_buf) {
            // exit CMD mode
            mode = Normal;
            curs_set(0);
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

    // handle serial data
    bool serial_received = false;

    // update buffer
    if ((cur_input != ERR) || win_resized || serial_received) {
      // clear screen
      clear();
      refresh();

      // update CMD, Status bar line
      int row = cur_height - 1;
      int col = 0;
      move(row, col);
      clrtoeol();
      switch (mode) {
        case Normal:
          // print status bar
          // [(O)pen/(C)losed] [(R)ecording/(nothing)] [COM_PORT] [BAUD]
          // [FORMAT] [ERROR_MESSAGE]
          printw("%c ", is_open ? 'O' : 'C');
          if (recording_to_file) {
            printw("%c ", 'R');
          }
          printw("%s %u %u-%u-%u ", port_name, baud, data_bits, start_bits,
                 parity_bit ? 1 : 0);

          if (cmd_error) {
            printw("%s", cmd_error_msg);
          }
          break;
        case Command_line:
          addch(':');
          move(row, ++col);
          printw("%s", cmd_buf);
          break;
        default:
            // do nothing
            ;
      }
    }

    // draw buffer
    refresh();
    napms(REFRESH_RATE_MS);
  }

  // end ncurses
  endwin();

  return 0;
}
