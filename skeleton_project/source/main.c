#include "hardware.h"
#include "requests.h"
#include "util.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define WAIT_TIME 1.0f

int start_elevator(State FSM[static 1], Request baseRequest[static 1]) {
  while (true) {
    FSM->current_floor = get_floor(FSM->current_floor);
    pollHardware(FSM, baseRequest);

    if (FSM->door_open) {
      while (fabs(difftime(FSM->timestamp, time(0))) <= WAIT_TIME) {
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
      if (doorCanOpen())
        handleAtFloor(FSM, baseRequest);

    } else if (requestToConsume(baseRequest))
      consumeRequest(FSM, baseRequest);
  }
  return 0;
}

int main() {
  if (hardware_init() != 0)
    exit(1);

  State *FSM = (State *)malloc(sizeof(State));
  Request *baseRequest = (Request *)malloc(sizeof(Request));
  FSM_init(FSM, baseRequest);

  return start_elevator(FSM, baseRequest);
}
