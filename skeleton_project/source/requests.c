#include "requests.h"
#include <stdbool.h>
#include <stdlib.h>

void insert_request_last(int floor, HardwareOrder orderType,
                         Request *base_req) {
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
};

void insert_request(int floor, HardwareOrder orderType, Request baseRequest[static 1],
                    State state[static 1]) {
  Request *currentRequest = baseRequest;
  while (currentRequest->child != NULL) {
    currentRequest = currentRequest->child;
    bool elevatorGoingUp = state->current_floor < state->going_to_floor;
    bool requestAboveCurrentGoal =
        state->going_to_floor < currentRequest->floor;

    if ((elevatorGoingUp && !requestAboveCurrentGoal &&
         currentRequest->orderType != HARDWARE_ORDER_DOWN) ||
        (!elevatorGoingUp && requestAboveCurrentGoal &&
         currentRequest->orderType != HARDWARE_ORDER_UP)) {
      Request *newRequest = (Request *)malloc(sizeof(Request));
      if ((newRequest != NULL)) {
        newRequest->floor = floor;
        newRequest->orderType = orderType;
        newRequest->parent = currentRequest->parent;
        newRequest->child = currentRequest;
        currentRequest->parent = newRequest;
        break;
      }
    }

  }

  // If at end of queue, insert
  if (currentRequest->child == NULL) {
    Request *newRequest = (Request *)malloc(sizeof(Request));
    if ((newRequest != NULL)) {
      newRequest->floor = floor;
      newRequest->orderType = orderType;
      newRequest->parent = currentRequest;
      newRequest->child = NULL;
      currentRequest->child = newRequest;
    }
  }
  printf("Queue length: %d", queueLength(baseRequest));
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
      free(current_request->child);
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