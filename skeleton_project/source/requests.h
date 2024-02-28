#ifndef ___REQUESTS
#define ___REQUESTS

#include "hardware.h"
#include "FSM.h"

typedef struct Request Request;
struct Request {
  int floor; // -1 indicates no request
  HardwareOrder orderType;
  Request *child;
  Request *parent;
};


void insert_request_last(int floor, HardwareOrder orderType, Request *base_req);
void insert_request(int floor, HardwareOrder orderType, Request *base_req, State *state);
void purge_requests(Request *base_req);
int queueLength(Request BaseRequest[static 1]);


// put your function headers here

#endif
