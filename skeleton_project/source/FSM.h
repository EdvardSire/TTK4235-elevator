#ifndef ___FSM
#define ___FSM

#include "hardware.h"
#include <sys/time.h>
#include <time.h>

#define NUMBER_OF_BUTTONS (HARDWARE_NUMBER_OF_FLOORS * 3)

typedef struct {
  int current_floor;
  int door_open;
  int moving;
  int stop;
  struct timeval tv[NUMBER_OF_BUTTONS];
  time_t timestamp; // seconds
} State;

#endif