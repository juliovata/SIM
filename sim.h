/*
  sim.h
  Julio Vata - jvata@umich.edu - 2024

*/

#ifndef SIM_H_
#define SIM_H_

typedef enum {
  Normal,
  Command_line
} program_mode;

// TODO(juliovata): How portable is this??
#define ESCAPE_KEY (27)

#endif
