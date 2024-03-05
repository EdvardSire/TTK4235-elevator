#ifndef ___REQUESTS
#define ___REQUESTS

#include "FSM.h"
#include "hardware.h"

typedef struct Request Request;
struct Request {
  int floor;
  Request *child;
  Request *parent;
};

void handleRequestLast(int floor, Request baseRequest[static 1]);


void handleRequest(State FSM[static 1], Request baseRequest[static 1], int floorRequest, HardwareOrder orderType);

void deleteAllRequest(Request baseRequest[static 1]);

void removeRequest(Request request[static 1]);

#endif
