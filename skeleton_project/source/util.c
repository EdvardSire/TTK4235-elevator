#include "util.h"
#include "debug.h"
#include <stdbool.h>
#include <stdio.h>

void FSM_init(State FSM[static 1], Request baseRequest[static 1]) {
  baseRequest->parent = NULL;
  baseRequest->child = NULL;
  FSM->current_floor = -1;
  FSM->stop = false;
  while (FSM->current_floor == -1) {
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
    FSM->current_floor = get_floor(FSM->current_floor);
  }
  handleAtFloor(FSM, baseRequest);

  for (int i = 0; i < NUMBER_OF_BUTTONS; i++)
    gettimeofday(&FSM->tv[i], NULL);

  DEBUG &&printf("Definied state established\n");
}

void clear_all_order_lights() {
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++)
    for (int order_type = 0; order_type < 3; order_type++)
      hardware_command_order_light(floor, order_type, 0);
}

void lights() {
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++)
    for (int order_type = 0; order_type < 3; order_type++)
      if (hardware_read_order(floor, order_type))
        hardware_command_order_light(floor, order_type, 1);
}

int get_floor(int lastFloor) {
  for (int i = 0; i < HARDWARE_NUMBER_OF_FLOORS; i++) {
    if (hardware_read_floor_sensor(i)) {
      hardware_command_floor_indicator_on(i);
      return i;
    }
  }
  return lastFloor;
}

int doorCanOpen() {
  for (int i = 0; i < HARDWARE_NUMBER_OF_FLOORS; i++) {
    if (hardware_read_floor_sensor(i)) {
      hardware_command_floor_indicator_on(i);
      return true;
    }
  }
  return false;
}

int compareTimeNow(struct timeval old[NUMBER_OF_BUTTONS], int index) {
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

void freeAtFloor(Request baseRequest[static 1]) {
  int floorToFree = baseRequest->child->floor;
  DEBUG &&printf("Request freed: %p\n", baseRequest->child);

  Request *current = baseRequest;
  while (current->child != NULL) {
    if (current->child->floor == floorToFree) {
      removeRequest(current->child);
      continue;
    }
    current = current->child;
    if (current == NULL)
      break;
  }
}

void handleAtFloor(State FSM[static 1], Request baseRequest[static 1]) {
  hardware_command_movement(HARDWARE_MOVEMENT_STOP);
  FSM->current_floor = get_floor(FSM->current_floor);
  FSM->moving = false;
  hardware_command_door_open(true);
  FSM->door_open = true;
  if (baseRequest->child != NULL)
    freeAtFloor(baseRequest);
  FSM->timestamp = time(0);

  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_INSIDE,
                               false);
  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_UP, false);
  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_DOWN, false);
}

void handleCloseDoor(State FSM[static 1]) {
  hardware_command_door_open(false);
  FSM->door_open = false;
  DEBUG &&printf("Door closed\n");
}

void handleEmergencyStop(State FSM[static 1], Request baseRequest[static 1]) {
  if (hardware_read_stop_signal()) {
    if (doorCanOpen()) {
      hardware_command_door_open(true);
      FSM->door_open = true;
    } else
      FSM->door_open = false;

    hardware_command_stop_light(true);
    hardware_command_movement(HARDWARE_MOVEMENT_STOP);
    FSM->moving = false;
    FSM->stop = true;
    deleteAllRequest(baseRequest);
    clear_all_order_lights();

    // Don't accept orders
    while (hardware_read_stop_signal()) {
      hardware_command_stop_light(true);
    }

    FSM->timestamp = time(0);
    hardware_command_stop_light(false);
  }
}

// HardwareOrder type is not used in the handleRequest function, but is there to
// bookkeep the timestamps for each button
void pollHardware(State FSM[static 1], Request baseRequest[static 1]) {
  handleEmergencyStop(FSM, baseRequest);
  int index = 0;
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++) {

    if (hardware_read_order(floor, HARDWARE_ORDER_DOWN) &&
        compareTimeNow(FSM->tv, index)) {
      handleRequest(FSM, baseRequest, floor, HARDWARE_ORDER_DOWN);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;

    if (hardware_read_order(floor, HARDWARE_ORDER_UP) &&
        compareTimeNow(FSM->tv, index)) {
      handleRequest(FSM, baseRequest, floor, HARDWARE_ORDER_UP);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;

    if (hardware_read_order(floor, HARDWARE_ORDER_INSIDE) &&
        compareTimeNow(FSM->tv, index)) {
      handleRequest(FSM, baseRequest, floor, HARDWARE_ORDER_INSIDE);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;
  }
  lights();
}
