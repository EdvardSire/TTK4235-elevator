#include "hardware.h"
#include "requests.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define DEBUG1 false
#define DEBUG2 true

#define DOUBLEMAGIC MAGIC+1

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

void freeRequest(Request * req) {
  if(req != NULL) {
  // NEVER GIVE BASE HERE
  Request * parent = req->parent;
  Request * child = req->child;
  free(req);
  parent->child = child;
  if(child != NULL)
    child->parent = parent;
  }
}

void handleAtFloor(State FSM[static 1], Request baseRequest[static 1]) {
  DEBUG2 ? printf("Length %d", queueLength(baseRequest)) : (void)0;
  hardware_command_movement(HARDWARE_MOVEMENT_STOP);
  FSM->current_floor = get_floor();
  FSM->moving = false;
  hardware_command_door_open(true);
  //test
  // floor_request_filled(FSM->current_floor, baseRequest);
  freeRequest(baseRequest->child);

  hardware_command_order_light(FSM->current_floor, HARDWARE_ORDER_INSIDE,
                               false);
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
  DEBUG1 ? printf("Consume\n") : (void)0;
  FSM->moving = true;
  if (baseRequest->child->floor > FSM->current_floor)
    hardware_command_movement(HARDWARE_MOVEMENT_UP);
  else
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
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
      //TODO POLL FOR REQUESTS
      hardware_command_stop_light(true);
      if(hardware_read_stop_signal())
        stop_timestamp = time(0);
    }
  }
}

int compareTimeNow(struct timeval old[MAGIC], int index) {
    // long POLL_TIMEOUT = 200; // milliseconds
    struct timeval end;
    gettimeofday(&end, NULL);

    double diff_sec = end.tv_sec - old[index].tv_sec;
    double diff_usec = end.tv_usec - old[index].tv_usec;
    double diff = diff_sec + (diff_usec / 1000000.0);

    if(diff >= 0.2f) {
    DEBUG1 ? printf("diff: %lf\n", diff) : (void)0;
      return true;
    }
    return false;
}


void pollHardware(State FSM[static 1], Request baseRequest[static 1]) {
  lights();
  int index = 0;
  for (int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++) {
    if (hardware_read_order(floor, HARDWARE_ORDER_UP) && compareTimeNow(FSM->tv, index)) {
      gettimeofday(&FSM->tv[index], NULL);
      insert_request_last(floor, HARDWARE_ORDER_UP, baseRequest);
      // insert_request(floor, HARDWARE_ORDER_UP, baseRequest, FSM);
    }
    index++;

    if (hardware_read_order(floor, HARDWARE_ORDER_INSIDE) && compareTimeNow(FSM->tv, index)) {
      gettimeofday(&FSM->tv[index], NULL);
      insert_request_last(floor, HARDWARE_ORDER_INSIDE, baseRequest);
      // insert_request(floor, HARDWARE_ORDER_INSIDE, baseRequest, FSM);
    }
    index++;

    if (hardware_read_order(floor, HARDWARE_ORDER_DOWN) && compareTimeNow(FSM->tv, index)) {
      gettimeofday(&FSM->tv[index], NULL);
      insert_request_last(floor, HARDWARE_ORDER_DOWN, baseRequest);
      // insert_request(floor, HARDWARE_ORDER_DOWN, baseRequest, FSM);
    }
    index++;
  }
  handleEmergencyStop(FSM, baseRequest);
}

int main() {
  if(hardware_init())
    exit(1);

  // Start Init
  State FSM = {};
  for(int i= 0; i < DOUBLEMAGIC; i++)
    gettimeofday(&FSM.tv[i], NULL);

  Request baseRequest = {.parent = NULL, .child = NULL};
  FSM.current_floor = get_floor();
  while (FSM.current_floor == -1) {
    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
    FSM.current_floor = get_floor();
  }
  handleAtFloor(&FSM, &baseRequest);
  // End Init

  // FSM.going_to_floor = 3;
  while (true) {
    FSM.current_floor = get_floor();
    hardware_command_stop_light(false);

    if (FSM.door_open) {
      DEBUG1 ? printf("Door open\n") : (void)0;
      while (fabs(difftime(FSM.timestamp, time(0))) <= 1.5f) {
        pollHardware(&FSM, &baseRequest);
        if (hardware_read_obstruction_signal())
          FSM.timestamp = time(0);
      }
      handleCloseDoor(&FSM);
      DEBUG1 ? printf("Door closed\n") : (void)0;
    } else
      pollHardware(&FSM, &baseRequest);

    if (FSM.moving) {
      DEBUG1 ? printf("Moving\n") : (void)0;
      while (FSM.current_floor != baseRequest.child->floor) {
        FSM.current_floor = get_floor();
        pollHardware(&FSM, &baseRequest);
      }
      handleAtFloor(&FSM, &baseRequest);
    } else if (requestToConsume(&baseRequest))
      consumeRequest(&FSM, &baseRequest);
  }
  return 0;
}
