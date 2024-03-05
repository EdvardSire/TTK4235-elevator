
#ifndef ___UTIL
#define ___UTIL

#include "FSM.h"
#include "hardware.h"
#include "requests.h"

void FSM_init(State FSM[static 1], Request baseRequest[static 1]);

void clear_all_order_lights();

void lights();

int get_floor(int lastFloor);

int doorCanOpen();

int compareTimeNow(struct timeval old[NUMBER_OF_BUTTONS], int index);

void freeAtFloor(Request baseRequest[static 1]);

void handleAtFloor(State FSM[static 1], Request baseRequest[static 1]);

void handleCloseDoor(State FSM[static 1]);

int requestToConsume(Request baseRequest[static 1]);

void consumeRequest(State FSM[static 1], Request baseRequest[static 1]);

void handleEmergencyStop(State FSM[static 1], Request baseRequest[static 1]);

void pollHardware(State FSM[static 1], Request baseRequest[static 1]);

#endif