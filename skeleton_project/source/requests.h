#ifndef ___REQUESTS
#define ___REQUESTS

#include "hardware.h"
#include "FSM.h"

typedef struct Request Request;
struct Request {
  int floor;
  HardwareOrder orderType;
  Request *child;
  Request *parent;
};

void insert_request_last(int floor, HardwareOrder orderType, Request base_req[static 1]);
void insert_request(int floor, HardwareOrder orderType, Request base_req[static 1], State state[static 1]);
void purge_requests(Request base_req[static 1]);
void floor_request_filled(int floor, Request base_req[static 1]);
int queueLength(Request BaseRequest[static 1]);

#endif
