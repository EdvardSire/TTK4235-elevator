#ifndef REQUESTS
#define REQUESTS

#include "hardware.h"

typedef struct Request Request;
struct Request {
  int floor; // -1 indicates no request
  HardwareOrder orderType;
  Request *child;
  Request *parent;
};

void insert_request_last(int floor, HardwareOrder orderType, Request *base_req);
void purge_requests();

// put your function headers here

#endif
