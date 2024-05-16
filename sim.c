/*
  sim.c
  Julio Vata - jvata@umich.edu - 2024

  TODO:   - deal with set_port empty string
          - implement set_port, set_baud, set_format, open, close
          - Handle terminal resize with interrupt
          - Deal with int to char conversion
          - CMD line should expand in height when command text passes edge
*/

#include "sim.h"

#include <getopt.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process_command.h"
#include "version.h"

// function prototypes
void print_help();
Cmd_arg_settings process_cmd_args();

// main function
int main(int argc, char *argv[]) {
  Cmd_arg_settings usr_cmd_args = process_cmd_args(argc, argv);

  // start ncurses
  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
  if (!usr_cmd_args.input_blocking_enabled) {
    nodelay(stdscr, TRUE);
  }

  curs_set(0);
  ESCDELAY = 0;  // program treats escape key like any other key

  printw("SIM v%d.%d.%d\n", MAJOR, MINOR, PATCH);
  printw("Julio Vata - jvata@umich.edu - 2024\n");
  refresh();
  bool welcome_on_screen = true;

  // program loop
  bool running = true;
  Program_mode mode = Normal;
  Program_mode prev_mode = Initial;

  char cmd_buf[CMD_BUF_SIZE] = {'\0'};
  char *cmd_write_ptr = cmd_buf;

  Program_error cur_error = No_error;

  int prev_width;
  int prev_height;

  bool is_open = false;
  char port_name[CMD_STRING_BUF_SIZE] = "NO_PORT";
  long baud = 0;
  unsigned int data_bits = 0;
  unsigned int start_bits = 0;
  bool parity_bit = 0;
  bool recording_to_file = false;

  // initialize previous screen dimensions
  getmaxyx(stdscr, prev_height, prev_width);

  // program loop
  while (running) {
    // clear screen if window resized
    int cur_width;
    int cur_height;
    getmaxyx(stdscr, cur_height, cur_width);

    // handle user input
    int cur_input = getch();
    if (cur_input != ERR) {
      if (welcome_on_screen) {
        // clear the welcome message
        move(0, 0);
        clrtoeol();
        move(1, 0);
        clrtoeol();
        welcome_on_screen = false;
      }

      if (mode == Normal) {
        switch (cur_input) {
          case ':':
            // switch to command line mode
            mode = Command_line;

            // reset command buffer
            cmd_write_ptr = cmd_buf;
            memset(cmd_buf, '\0', CMD_BUF_SIZE);

            // reset error handling
            cur_error = No_error;

            // enable cursor
            curs_set(1);
            break;
          case 'q':
            // quit program (just in case its needed)
            running = false;
          default:
              // do nothing
              ;
        }
      } else if (mode == Command_line) {
        if (cur_input == ESCAPE_KEY) {
          // leave CMD mode and disable cursor
          mode = Normal;
          curs_set(0);
        } else if (cur_input == '\n') {  // enter key
          // execute command, exit command line mode
          mode = Normal;
          curs_set(0);

          if (cmd_write_ptr != cmd_buf) {
            // process non empty command
            Processed_command cur_cmd_data;
            process_command(&cur_cmd_data, cmd_buf);

            switch (cur_cmd_data.type) {
              case Err_parse:
                cur_error = Command_parse;
                break;
              case Err_arg:
                cur_error = Command_argument;
                break;
              case Err_unknown:
                cur_error = Command_unknown;
                break;
              case Quit:
                running = false;
                break;
              case Set_port:
                strcpy(port_name, cur_cmd_data.string_args[0]);
                break;
              case Set_baud:
                baud = cur_cmd_data.numeric_args[0];
                if (baud < 0) {
                  baud = 0;
                  cur_error = Command_argument;
                }
                break;
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
        }
#ifdef KEY_RESIZE
        // this bit of code here basically disables KEY_RESIZE which based on
        // my understanding is not particularly portable
        else if (cur_input == KEY_RESIZE) {
          // do nothing
        }
#endif
        else if (cmd_write_ptr < (cmd_buf + CMD_BUF_SIZE - 1)) {
          // add input to buffer if there is space
          // TODO(juliovata):   what to do about this conversion here?
          //                    only add ascii characters to buffer?
          *cmd_write_ptr = cur_input;
          ++cmd_write_ptr;
        }
      }
    }

    // update main window
    if (running) {
      // handle serial data
      bool serial_received = false;

      bool win_resized =
          (cur_width != prev_width) || (cur_height != prev_height);
      if (win_resized) {
        clear();
        refresh();
        napms(REFRESH_RATE_MS);
      }

      // update serial data portion of window
      if (serial_received) {
        // do writing to proper portion of screen
      }

      // update command line/status bar portion of window
      int row = cur_height - 1;
      int col = 0;
      if ((mode == Command_line) && ((cur_input != ERR) || win_resized)) {
        // update command line if there was user input
        // update CMD, Status bar line
        move(row, col);
        clrtoeol();
        addch(':');
        move(row, ++col);
        printw("%s", cmd_buf);
      } else if (((mode == Normal) &&
                  ((prev_mode == Command_line) || win_resized)) ||
                 (prev_mode == Initial)) {
        // update status bar after leaving command line mode or at the start of
        // program execution
        // [(O)pen/(C)losed] [(R)ecording/(nothing)] [COM_PORT] [BAUD]
        // [FORMAT] [ERROR_MESSAGE]
        move(row, col);
        clrtoeol();
        printw("%c ", is_open ? 'O' : 'C');
        if (recording_to_file) {
          printw("%c ", 'R');
        }
        printw("%s %lu %u-%u-%u ", port_name, baud, data_bits, start_bits,
               parity_bit ? 1 : 0);
        switch (cur_error) {
          case Command_unknown:
            printw("%s", "COMMAND NOT FOUND");
            break;
          case Command_argument:
            printw("%s", "INCORRECT COMMAND ARGUMENTS");
            break;
          case Command_parse:
            printw("%s", "COMMAND PARSE ERROR");
            break;
          case No_error:
          default:
              // do nothing
              ;
        }
      }
    }

    prev_mode = mode;
    prev_width = cur_width;
    prev_height = cur_height;

    // draw buffer
    refresh();
    napms(REFRESH_RATE_MS);
  }

  // end ncurses
  endwin();

  return 0;
}

void print_help() {
  printf("SIM v%d.%d.%d\n", MAJOR, MINOR, PATCH);
  printf("Usage: sim [arguments]\n");
  printf("arguments:\n");
  printf("\t--h or --help\t\tPrint Help (this message) and exit\n");
  printf(
      "\t--b or --blocking-input\t\tEnable blocking input (mostly for "
      "debugging purposes\n");
}

Cmd_arg_settings process_cmd_args(int argc, char *argv[]) {
  opterr = false;
  int choice;
  int option_index = 0;
  struct option long_options[] = {{"blocking-input", no_argument, NULL, 'b'},
                                  {"help", no_argument, NULL, 'h'},
                                  {NULL, 0, NULL, '\0'}};

  bool enable_input_blocking = false;

  while ((choice = getopt_long(argc, argv, "bh", long_options,
                               &option_index)) != -1) {
    switch (choice) {
      case 'b':
        enable_input_blocking = true;
        break;
      case 'h':
        print_help();
        exit(1);
      default:
        printf("SIM v%d.%d.%d\n", MAJOR, MINOR, PATCH);
        printf("Unknown option argument\n");
        printf("More info with: \"sim -h\"\n");
        exit(1);
    }
  }

  return (Cmd_arg_settings){enable_input_blocking};
}
