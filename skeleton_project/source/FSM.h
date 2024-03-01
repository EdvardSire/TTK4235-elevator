#ifndef ___FSM
#define ___FSM

#include "hardware.h"
#include <sys/time.h>
#include <time.h>

#define MAGIC ((HARDWARE_NUMBER_OF_FLOORS * 3) - 2)

typedef struct {
  int current_floor;
  int door_open;
  int moving;
  int direction_up;
  struct timeval tv[MAGIC];
  time_t timestamp; // seconds
} State;

#endif