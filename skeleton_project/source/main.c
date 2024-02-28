#include "hardware.h"
#include "requests.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void msleep(long msec) {
  struct timespec ts;
  int res;

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  nanosleep(&ts, &ts);
}

static void clear_all_order_lights() {
  HardwareOrder order_types[3] = {HARDWARE_ORDER_UP, HARDWARE_ORDER_INSIDE,
                                  HARDWARE_ORDER_DOWN};

  for (int f = 0; f < HARDWARE_NUMBER_OF_FLOORS; f++) {
    for (int i = 0; i < 3; i++) {
      HardwareOrder type = order_types[i];
      hardware_command_order_light(f, type, 0);
    }
  }
}

static void lights() {
  /* All buttons must be polled, like this: */
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++) {
    if (hardware_read_floor_sensor(floor)) {
      hardware_command_floor_indicator_on(floor);
    }
  }

  /* Lights are set and cleared like this: */
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++) {
    /* Internal orders */
    if (hardware_read_order(floor, HARDWARE_ORDER_INSIDE)) {
      hardware_command_order_light(floor, HARDWARE_ORDER_INSIDE, 1);
    }

    /* Orders going up */
    if (hardware_read_order(floor, HARDWARE_ORDER_UP)) {
      hardware_command_order_light(floor, HARDWARE_ORDER_UP, 1);
    }

    /* Orders going down */
    if (hardware_read_order(floor, HARDWARE_ORDER_DOWN)) {
      hardware_command_order_light(floor, HARDWARE_ORDER_DOWN, 1);
    }
  }
}

static int get_floor() {
  for (int i = 0; i < HARDWARE_NUMBER_OF_FLOORS; i++)
    if (hardware_read_floor_sensor(i)) {
      hardware_command_floor_indicator_on(i);
      return i;
    }
  return -1;
}

void handleAtFloor(State FSM[static 1]) {
  hardware_command_movement(HARDWARE_MOVEMENT_STOP);
  FSM->current_floor = get_floor();
  FSM->moving = false;
  hardware_command_door_open(true);
  hardware_command_order_light(FSM->current_floor,  HARDWARE_ORDER_INSIDE, false);
  FSM->door_open = true;
  FSM->timestamp = time(0);
}

void handleCloseDoor(State FSM[static 1]) {
  hardware_command_door_open(false);
  FSM->door_open = false;
}

Request *requestToConsume(Request BaseRequest[static 1]) {
  if (BaseRequest->child == NULL)
    return NULL;
  return BaseRequest->child;
}


void consumeRequest(State FSM[static 1], Request request[static 1],
                    Request BaseRequest[static 1]) {

  FSM->moving = true;
  FSM->going_to_floor = request->floor;
  if (FSM->going_to_floor > FSM->current_floor)
    hardware_command_movement(HARDWARE_MOVEMENT_UP);
  else
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);

  if (request->child == NULL) {
    free(request);
    BaseRequest->child = NULL;
  } else {
    BaseRequest->child = request->child;
    free(request);
  }
}

void pollRequest(Request BaseRequest[static 1], State FSM[static 1]) {
  lights();
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++) {
    if (hardware_read_order(floor, HARDWARE_ORDER_DOWN)) {
      insert_request(floor, HARDWARE_ORDER_DOWN, BaseRequest, FSM);
      msleep(200);
    }
    if (hardware_read_order(floor, HARDWARE_ORDER_UP)) {
      insert_request(floor, HARDWARE_ORDER_UP, BaseRequest, FSM);
      msleep(200);
    }
    if (hardware_read_order(floor, HARDWARE_ORDER_INSIDE)) {
      insert_request(floor, HARDWARE_ORDER_INSIDE, BaseRequest, FSM);
      msleep(200);
    }
  }
}

int main() {
  if (hardware_init() != 0) {
    fprintf(stderr, "Unable to initialize hardware\n");
    exit(1);
  }
  printf("Press the stop button on the elevator panel to exit\n");

  // Start Init
  State FSM;
  Request BaseRequest = {.parent = NULL, .child = NULL};
  FSM.current_floor = get_floor();
  while (FSM.current_floor == -1) {
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
    FSM.current_floor = get_floor();
  }
  handleAtFloor(&FSM);
  // End Init

  // FSM.going_to_floor = 3;
  while (true) {
    FSM.current_floor = get_floor();
    if (!hardware_read_stop_signal()) {
      hardware_command_stop_light(false);

      if (FSM.door_open) {
        printf("Door open\n");
        while (fabs(difftime(FSM.timestamp, time(0))) <= 3.0f) {
          pollRequest(&BaseRequest, &FSM);
          if (hardware_read_obstruction_signal())
            FSM.timestamp = time(0);
        }
        handleCloseDoor(&FSM);
        printf("Door closed\n");
      } else {
        pollRequest(&BaseRequest, &FSM);
      }

      if (FSM.moving) {
        printf("Moving\n");
        while (FSM.current_floor != FSM.going_to_floor) {
          FSM.current_floor = get_floor();
          pollRequest(&BaseRequest, &FSM);
        }
        handleAtFloor(&FSM);
      } else {
        Request *possibleRequest = requestToConsume(&BaseRequest);
        if (possibleRequest != NULL)
          consumeRequest(&FSM, possibleRequest, &BaseRequest);

        lights();
      }

    } else { // if stop signal
      hardware_command_stop_light(true);
    }
    lights();
  }

  return 0;
}
