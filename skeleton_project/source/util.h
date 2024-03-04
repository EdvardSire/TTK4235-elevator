
#ifndef ___UTIL
#define ___UTIL

#include "FSM.h"
#include "hardware.h"

int compareTimeNow(struct timeval old[MAGIC], int index);
void clear_all_order_lights();
void lights();
int get_floor(int lastFloor);
int doorCanOpen();

#endif