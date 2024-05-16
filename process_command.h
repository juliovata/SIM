/*
  process_command.h
  Julio Vata - jvata@umich.edu - 2024
*/

#ifndef PROCESS_COMMAND_H_
#define PROCESS_COMMAND_H_

#define CMD_STRING_BUF_SIZE (64)
#define CMD_NUM_ARG_LIMIT (1)
#define CMD_STR_ARG_LIMIT (1)

typedef enum {
  Err_parse,
  Err_arg,
  Err_unknown,
  Quit,
  Set_port,
  Set_baud
} Command_type;

// Argument path
// n - num, s - string, p - path (string with spaces)
typedef struct {
  const char* command;
  const char* argument_format;
  unsigned int num_args;
  Command_type type;
} Command;

typedef struct {
  long numeric_args[CMD_NUM_ARG_LIMIT];
  Command_type type;
  unsigned int num_numeric_args;
  unsigned int num_string_args;
  char string_args[CMD_STR_ARG_LIMIT][CMD_STRING_BUF_SIZE];
} Processed_command;

void process_command(Processed_command*, char*);

#endif
