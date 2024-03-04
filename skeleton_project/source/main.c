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

#define DEBUG true

void freeAtFloorOld(Request baseRequest[static 1]) {
  Request *childChild = baseRequest->child->child;
  DEBUG &&printf("Request freed: %p\n", baseRequest->child);
  free(baseRequest->child);
  if (childChild != NULL) {
    childChild->parent = baseRequest;
    baseRequest->child = childChild;
  } else
    baseRequest->child = NULL;
}

void freeAtFloor(Request baseRequest[static 1]) {
  int floorToFree = baseRequest->child->floor;

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

int requestToConsume(Request baseRequest[static 1]) {
  if (baseRequest->child == NULL)
    return false;

  DEBUG &&printf("RequestToConsume\n");
  return true;
}

void consumeRequest(State FSM[static 1], Request baseRequest[static 1]) {
  FSM->moving = true;
  if (baseRequest->child->floor > FSM->current_floor) {
    hardware_command_movement(HARDWARE_MOVEMENT_UP);
    DEBUG &&printf("Moving up\n");
  } else {
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
    DEBUG &&printf("Moving down\n");
  }
}

void handleEmergencyStop(State FSM[static 1], Request baseRequest[static 1]) {
  if (hardware_read_stop_signal()) {
    if (doorCanOpen()) {
      hardware_command_door_open(true);
      FSM->door_open = true;
    } else
      FSM->door_open = false;
    hardware_command_movement(HARDWARE_MOVEMENT_STOP);
    FSM->moving = false;
    FSM->timestamp = time(0);
    FSM->stop = true;
    deleteAllRequest(baseRequest);
    clear_all_order_lights();

    // Don't accept orders
    while (hardware_read_stop_signal()) {
      hardware_command_stop_light(true);
    }
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
      INSERT_LAST
      ? handleRequestLast(floor, baseRequest)
      : handleRequest(floor, baseRequest, FSM);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;

    if (hardware_read_order(floor, HARDWARE_ORDER_UP) &&
        compareTimeNow(FSM->tv, index)) {
      INSERT_LAST
      ? handleRequestLast(floor, baseRequest)
      : handleRequest(floor, baseRequest, FSM);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;

    if (hardware_read_order(floor, HARDWARE_ORDER_INSIDE) &&
        compareTimeNow(FSM->tv, index)) {
      INSERT_LAST
      ? handleRequestLast(floor, baseRequest)
      : handleRequest(floor, baseRequest, FSM);
      gettimeofday(&FSM->tv[index], NULL);
    }
    index++;
  }
  lights();
}

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

  for (int i = 0; i < MAGIC; i++)
    gettimeofday(&FSM->tv[i], NULL);

  DEBUG &&printf("Definied state established\n");
}

int main() {
  if (hardware_init() != 0)
    exit(1);

  // software init
  State *FSM = (State *)malloc(sizeof(State));
  Request *baseRequest = (Request *)malloc(sizeof(Request));
  FSM_init(FSM, baseRequest);
  // software init

  while (true) {
    FSM->current_floor = get_floor(FSM->current_floor);
    pollHardware(FSM, baseRequest);
    if (FSM->door_open) {
      while (fabs(difftime(FSM->timestamp, time(0))) <= 1.0f) {
        pollHardware(FSM, baseRequest);
        if (hardware_read_obstruction_signal())
          FSM->timestamp = time(0);
      }
      handleCloseDoor(FSM);
    }

    if (FSM->moving) {
      FSM->stop = false;
      while (!FSM->stop && (FSM->current_floor != baseRequest->child->floor)) {
        FSM->current_floor = get_floor(FSM->current_floor);
        pollHardware(FSM, baseRequest);
      }
      if(doorCanOpen())
        handleAtFloor(FSM, baseRequest);

    } else if (requestToConsume(baseRequest))
      consumeRequest(FSM, baseRequest);

  }
  return 0;
}
