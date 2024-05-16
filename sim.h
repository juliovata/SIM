/*
  sim.h
  Julio Vata - jvata@umich.edu - 2024

*/

#ifndef SIM_H_
#define SIM_H_

#include <stdbool.h>

#define REFRESH_RATE_MS (16)

#define CMD_BUF_SIZE (1024)
#define ERROR_MSG_BUF_SIZE (64)

typedef enum { Normal, Command_line, Initial } Program_mode;
typedef enum {
  No_error,
  Command_unknown,
  Command_argument,
  Command_parse
} Program_error;

typedef struct {
  bool input_blocking_enabled;
} Cmd_arg_settings;

// TODO(juliovata): How portable is this??
#define ESCAPE_KEY (27)

#endif
