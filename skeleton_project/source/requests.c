#include "requests.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

void _free_request(Request request[static 1]) {
  if (request->child == NULL) {
    Request *parent = request->parent;
    free(parent->child); // request
    parent->child = NULL;
  } else {
    Request *parent = request->parent;
    Request *child = request->child;
    free(parent->child); // request
    parent->child = child;
    child->parent = parent;
  }
}

void _new_request(Request newRequest[static 1], Request parent[static 1],
                  int floor, HardwareOrder orderType) {
  if (newRequest == NULL)
    exit(0);
  newRequest->floor = floor;
  newRequest->orderType = orderType;
  newRequest->parent = parent;
  newRequest->child = parent->child; // important this first
  parent->child = newRequest;
  printf("New request, floor: %d \n", floor);
}

void insert_request_last(int floor, HardwareOrder orderType, Request base_req[static 1]) {
  Request *current_request = base_req;
  while (current_request->child != NULL)
    current_request = current_request->child;

  Request *new_req = (Request *)malloc(sizeof(Request));
  if ((new_req != NULL)) {
    new_req->floor = floor;
    new_req->orderType = orderType;
    new_req->parent = current_request;
    new_req->child = NULL;
    current_request->child = new_req;
  }
  printf("Queue length: %d \n", queueLength(base_req));
};

void insert_request(int floor, HardwareOrder orderType,
                    Request baseRequest[static 1], State state[static 1]) {
  Request *currentRequest = baseRequest;
  while (currentRequest->child != NULL) {
    currentRequest = currentRequest->child;
    bool elevatorGoingUp = state->current_floor < baseRequest->child->floor;
    bool requestAboveCurrentGoal =
        baseRequest->child->floor < currentRequest->floor;

    if ((elevatorGoingUp && !requestAboveCurrentGoal &&
         (currentRequest->orderType != HARDWARE_ORDER_DOWN) && !(state->current_floor > floor)) ||
        (!elevatorGoingUp && requestAboveCurrentGoal &&
         (currentRequest->orderType != HARDWARE_ORDER_UP) && !(state->current_floor < floor))) {
      Request *newRequest = (Request *)malloc(sizeof(Request));
      _new_request(newRequest, currentRequest->parent, floor, orderType);
      break;
    }
  }

  // If at end of queue, insert
  if (currentRequest->child == NULL) {
    Request *newRequest = (Request *)malloc(sizeof(Request));
    _new_request(newRequest, currentRequest, floor, orderType);
    printf("Inserted end\n");
  }
  printf("Queue length: %d \n", queueLength(baseRequest));
}

void purge_requests(Request base_req[static 1]) {
  Request *current_request = base_req;
  while (current_request->child != NULL)
    current_request = current_request->child;

  while (current_request->parent != NULL) {
    current_request = current_request->parent;
    free(current_request->child);
  }
  base_req->child = NULL;
}

void floor_request_filled(int floor, Request base_req[static 1]) {
  Request *current_request = base_req;
  // Traverse LL to bottom
  while (current_request->child != NULL)
    current_request = current_request->child;

  while (current_request->parent != NULL) {
    current_request = current_request->parent;
    if (current_request->child->floor == floor)
      _free_request(current_request->child);
  }
}

int queueLength(Request BaseRequest[static 1]) {
  int sum = 0;
  Request *current_request = BaseRequest;
  while (current_request->child != NULL) {
    current_request = current_request->child;
    sum += 1;
  }

  return sum;
}



