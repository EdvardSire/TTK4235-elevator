#ifndef ___FSM
#define ___FSM

#include <time.h>

typedef struct {
  int current_floor;
  int going_to_floor;
  int door_open;
  int moving;
  time_t timestamp; // seconds
} State;

#endif