#include "util.h"
#include "hardware.h"
#include <stdbool.h>

int compareTimeNow(struct timeval old[MAGIC], int index) {
  struct timeval end;
  gettimeofday(&end, NULL);

  double diff_sec = end.tv_sec - old[index].tv_sec;
  double diff_usec = end.tv_usec - old[index].tv_usec;
  double diff = diff_sec + (diff_usec / 1000000.0);

  double THRESHOLD = 0.2f;
  if (diff >= THRESHOLD) {
    return true;
  }
  return false;
}

void clear_all_order_lights() {
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++)
    for (int order_type = 0; order_type < 3; order_type++)
      hardware_command_order_light(floor, order_type, 0);
}

void lights() {
  /* Lights are set and cleared like this: */
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++)
    for (int order_type = 0; order_type < 3; order_type++)
      if (hardware_read_order(floor, order_type))
        hardware_command_order_light(floor, order_type, 1);
}

int get_floor() {
  for (int i = 0; i < HARDWARE_NUMBER_OF_FLOORS; i++) {
    if (hardware_read_floor_sensor(i)) {
      hardware_command_floor_indicator_on(i);
      return i;
    }
  }
  return -1;
}