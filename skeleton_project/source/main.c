#include "hardware.h"
#include "requests.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void clear_all_order_lights() {
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++)
    for (int order_type = 0; order_type < 3; order_type++)
      hardware_command_order_light(floor, order_type, 0);
}

static void lights() {
  /* Lights are set and cleared like this: */
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++)
    for (int order_type = 0; order_type < 3; order_type++)
      if (hardware_read_order(floor, order_type))
        hardware_command_order_light(floor, order_type, 1);
}

static int get_floor() {
  for (int i = 0; i < HARDWARE_NUMBER_OF_FLOORS; i++) {
    if (hardware_read_floor_sensor(i)) {
      hardware_command_floor_indicator_on(i);
      return i;
    }
  }
  return -1;
}

void handleAtFloor(State FSM[static 1], Request baseRequest[static 1]) {
  hardware_command_movement(HARDWARE_MOVEMENT_STOP);
  FSM->current_floor = get_floor();
  FSM->previous_floor = FSM->current_floor;
  FSM->moving = false;
  hardware_command_door_open(true);
  floor_request_filled(FSM->current_floor, baseRequest);
  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_INSIDE, false);
  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_UP, false);
  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_DOWN, false);
  FSM->door_open = true;
  FSM->timestamp = time(0);
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
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++) {
    if (hardware_read_order(floor, HARDWARE_ORDER_DOWN))
      insert_request(floor, HARDWARE_ORDER_DOWN, baseRequest, FSM);

    if (hardware_read_order(floor, HARDWARE_ORDER_UP))
      insert_request(floor, HARDWARE_ORDER_UP, baseRequest, FSM);

    if (hardware_read_order(floor, HARDWARE_ORDER_INSIDE))
      insert_request(floor, HARDWARE_ORDER_INSIDE, baseRequest, FSM);
  }
  handleEmergencyStop(FSM, baseRequest);
}

void FSM_init(State FSM[static 1], Request baseRequest[static 1]) {
  baseRequest->parent = NULL;
  baseRequest->child = NULL;
  FSM->current_floor = get_floor();
  while (FSM->current_floor == -1) {
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
    FSM->current_floor = get_floor();
  }
  handleAtFloor(FSM, baseRequest);
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
    FSM->current_floor = get_floor();
    if (FSM->door_open) {
      while (fabs(difftime(FSM->timestamp, time(0))) <= 1.5f) {
        pollHardware(FSM, baseRequest);
        if (hardware_read_obstruction_signal())
          FSM->timestamp = time(0);
      }
      handleCloseDoor(FSM);
    } else
      pollHardware(FSM, baseRequest);

    if (FSM->moving) {
      while (FSM->current_floor != baseRequest->child->floor) {
        FSM->current_floor = get_floor();
        pollHardware(FSM, baseRequest);
      }
      handleAtFloor(FSM, baseRequest);
    } else if (requestToConsume(baseRequest))
      consumeRequest(FSM, baseRequest);
  }
  return 0;
}
