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

void insert_request_last(int floor, HardwareOrder orderType,
                         Request baseRequest[static 1]);

void handleRequest(int floor, Request baseRequest[static 1],
                   State FSM[static 1]);

void purge_requests(Request base_req[static 1]);

void floor_request_filled(int floor, Request base_req[static 1]);

int queueLength(Request BaseRequest[static 1]);

void _dumpQueue(Request baseRequest[static 1]);

void _free_request(Request request[static 1]);

void _new_request(Request newRequest[static 1], Request parent[static 1],
                  int floor, HardwareOrder orderType);

#endif
