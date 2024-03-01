#ifndef ___FSM
#define ___FSM

#include <time.h>

typedef struct {
  int current_floor;
  int door_open;
  int moving;
  int direction_up;
  time_t timestamp; // seconds
} State;

#endif