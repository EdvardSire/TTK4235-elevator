#include "hardware.h"
#include "requests.h"
#include "util.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define DOUBLEMAGIC MAGIC + 1

#define INSERT_LAST false

void freeAtFloor(Request baseRequest[static 1]) {
  Request *childChild = baseRequest->child->child;
  free(baseRequest->child);
  if (childChild != NULL) {
    childChild->parent = baseRequest;
    baseRequest->child = childChild;
  } else
    baseRequest->child = NULL;
}

void handleAtFloor(State FSM[static 1], Request baseRequest[static 1]) {
  hardware_command_movement(HARDWARE_MOVEMENT_STOP);
  FSM->current_floor = get_floor(FSM->current_floor);
  FSM->previous_floor = FSM->current_floor;
  FSM->moving = false;
  hardware_command_door_open(true);
  FSM->moving = false;
  if (baseRequest->child != NULL)
    freeAtFloor(baseRequest);
  FSM->door_open = true;
  FSM->timestamp = time(0);

  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_INSIDE,
                               false);
  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_UP, false);
  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_DOWN, false);
}

void handleCloseDoor(State FSM[static 1]) {
  hardware_command_door_open(false);
  FSM->door_open = false;
}

int requestToConsume(Request baseRequest[static 1]) {
  if (baseRequest->child == NULL)
    return false;
  return true;
}

void consumeRequest(State FSM[static 1], Request baseRequest[static 1]) {
  FSM->moving = true;
  if (baseRequest->child->floor > FSM->current_floor) {
    hardware_command_movement(HARDWARE_MOVEMENT_UP);
    FSM->direction_up = true;
  } else {
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
    FSM->direction_up = false;
  }
}

void handleEmergencyStop(State FSM[static 1], Request baseRequest[static 1]) {
  time_t stop_timestamp;
  if (hardware_read_stop_signal()) {
    hardware_command_movement(HARDWARE_MOVEMENT_STOP);
    FSM->moving = false;
    purge_requests(baseRequest);
    clear_all_order_lights();
    stop_timestamp = time(0);
    while (fabs(difftime(stop_timestamp, time(0))) <= 3.0f) {
      // TODO POLL FOR REQUESTS
      hardware_command_stop_light(true);
      if (hardware_read_stop_signal())
        stop_timestamp = time(0);
    }
  }
}

void pollHardware(State FSM[static 1], Request baseRequest[static 1]) {
  lights();
  int index = 0;
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++) {

    if (hardware_read_order(floor, HARDWARE_ORDER_DOWN) &&
        compareTimeNow(FSM->tv, index)) {
      INSERT_LAST
          ? insert_request_last(floor, HARDWARE_ORDER_DOWN, baseRequest)
          : handleRequest(floor, baseRequest, FSM);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;

    if (hardware_read_order(floor, HARDWARE_ORDER_UP) &&
        compareTimeNow(FSM->tv, index)) {
      INSERT_LAST
          ? insert_request_last(floor, HARDWARE_ORDER_UP, baseRequest)
          : handleRequest(floor, baseRequest, FSM);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;

    if (hardware_read_order(floor, HARDWARE_ORDER_INSIDE) &&
        compareTimeNow(FSM->tv, index)) {
      INSERT_LAST
          ? insert_request_last(floor, HARDWARE_ORDER_INSIDE, baseRequest)
          : handleRequest(floor, baseRequest, FSM);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;
  }
  handleEmergencyStop(FSM, baseRequest);
}

void FSM_init(State FSM[static 1], Request baseRequest[static 1]) {
  baseRequest->parent = NULL;
  baseRequest->child = NULL;
  FSM->current_floor = -1;
  while (FSM->current_floor == -1) {
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
    FSM->current_floor = get_floor(FSM->current_floor);
  }
  handleAtFloor(FSM, baseRequest);

  for (int i = 0; i < DOUBLEMAGIC + 1; i++)
    gettimeofday(&FSM->tv[i], NULL);
}

int main() {
  if (hardware_init() != 0)
    exit(1);

  // software init
  State *FSM = (State *)malloc(sizeof(Request));
  Request *baseRequest = (Request *)malloc(sizeof(Request));
  FSM_init(FSM, baseRequest);
  // software init

  while (true) {
    FSM->current_floor = get_floor(FSM->current_floor);
    if (FSM->door_open) {
      while (fabs(difftime(FSM->timestamp, time(0))) <= 1.0f) {
        pollHardware(FSM, baseRequest);
        if (hardware_read_obstruction_signal())
          FSM->timestamp = time(0);
      }
      handleCloseDoor(FSM);
    } else
      pollHardware(FSM, baseRequest);

    if (FSM->moving) {
      while (FSM->current_floor != baseRequest->child->floor) {
        FSM->current_floor = get_floor(FSM->current_floor);
        pollHardware(FSM, baseRequest);
      }
      handleAtFloor(FSM, baseRequest);
    } else if (requestToConsume(baseRequest))
      consumeRequest(FSM, baseRequest);
  }
  return 0;
}
