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

void handleRequest(int floor, Request baseRequest[static 1],
                   State FSM[static 1]);

#endif
