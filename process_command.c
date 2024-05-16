/*
  command_handler.c
  Julio Vata - jvata@umich.edu - 2024
*/

#include "process_command.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const Command command_table[] = {{"q", "", 0, Quit},
                                 {"set_port", "p", 1, Set_port},
                                 {NULL, NULL, 0, Err_unknown}};

// assumes cmd_info not empty
void process_command(Processed_command *processed_cmd, char *cmd_buf) {
  // tokenize buffer input
  const char *tok1 = strtok(cmd_buf, " ");
  if (tok1 == NULL) {
    processed_cmd->type = Err_parse;
    return;
  }

  // determine which command we are currently dealing with
  // TODO: this might be better handled with a hash table...
  const Command *cur_command = command_table;
  while ((cur_command->type != Err_unknown) &&
         (strcmp(tok1, cur_command->command) != 0)) {
    ++cur_command;
  }
  processed_cmd->type = cur_command->type;

  // return if processed_cmd is unknown
  if (processed_cmd->type == Err_unknown) {
    return;
  }

  /*
    handle arguments
  */
  processed_cmd->num_string_args = 0;
  processed_cmd->num_numeric_args = 0;

  // special case where argument is file path
  // assume the rest of the command is the argument
  if ((cur_command->num_args == 1) &&
      (cur_command->argument_format[0] == 'p')) {
    const char *path_arg = cmd_buf + strlen(tok1) + 1;
    const unsigned long path_len = strlen(path_arg);

    // if path is to long to store in buffer return or there is no argument
    if ((path_len == 0) || (path_len > (CMD_STRING_BUF_SIZE - 1))) {
      processed_cmd->type = Err_arg;
      return;
    }

    // copy path into processed_cmd if it fits
    strcpy(processed_cmd->string_args[0], path_arg);
    processed_cmd->num_string_args = 1;
    processed_cmd->num_numeric_args = 0;
  } else {
    // handle arguments as space deliminated tokens
    processed_cmd->num_string_args = 0;
    processed_cmd->num_numeric_args = 0;

    const char *cur_arg_format = cur_command->argument_format;
    unsigned int cur_num_args = 0;
    const char *next_tok;
    while ((next_tok = strtok(0, " ")) && (*next_tok != '\0')) {
      ++cur_num_args;
      if (cur_num_args > cur_command->num_args) {
        // too many arguments so end early
        processed_cmd->type = Err_arg;
        return;
      }

      // deal with current token based on current format
      if (*cur_arg_format == 's') {
        // check to see if argument will fit in buffer and not empty
        const unsigned long arg_len = strlen(next_tok);
        if ((arg_len == 0) || (arg_len > (CMD_STRING_BUF_SIZE - 1))) {
          processed_cmd->type = Err_arg;
          return;
        }

        // copy argument into buffer
        strcpy(processed_cmd->string_args[processed_cmd->num_string_args],
               next_tok);
        ++(processed_cmd->num_string_args);
      } else if (*cur_arg_format == 'n') {
        // confirm argument is number
        const char *c = next_tok;
        if (*c == '-') {
          ++c;
        }
        for (; *c != '\0'; ++c) {
          if (!isdigit(*c)) {
            processed_cmd->type = Err_arg;
            return;
          }
        }

        // use atoi
        processed_cmd->numeric_args[processed_cmd->num_numeric_args] =
            atoi(next_tok);
        ++(processed_cmd->num_numeric_args);
      }

      ++cur_arg_format;
    }

    if (cur_num_args !=
        processed_cmd->num_string_args + processed_cmd->num_numeric_args) {
      // wrong number of arguments
      processed_cmd->type = Err_arg;
      return;
    }
  }
}
