#include "requests.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


void _dumpQueue(Request baseRequest[static 1]) {
  Request * current = baseRequest;
  while(current->child != NULL) {
    printf("%d ", current->child->floor);
    current = current->child;
  }
  printf("\n");
}

void _free_request(Request request[static 1]) {
  Request *parent = request->parent;
  if (request->child == NULL) { // At bottom
    parent->child = NULL;
  } 
  else {
    Request *child = request->child;
    parent->child = child;
    child->parent = parent;
  }
  free(request); // request
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
  // printf("New request, floor: %d \n", floor);
}

void insert_request_last(int floor, HardwareOrder orderType, Request baseRequest[static 1]) {
  Request *current_request = baseRequest;
  // Get bottom request
  while (current_request->child != NULL)
    current_request = current_request->child;

  // Create the new one
  Request *new = (Request *)malloc(sizeof(Request));
  new->parent = current_request;
  new->child = NULL;
  new->floor = floor;
  new->orderType = orderType;

  // Update the bottom one
  current_request->child = new;

  // Debug
  _dumpQueue(baseRequest);
};

void insert_request(int floor, HardwareOrder orderType,
                    Request baseRequest[static 1], State state[static 1]) {
  Request *currentRequest = baseRequest;
   

  while (currentRequest->child != NULL) {
    currentRequest = currentRequest->child;
    if(currentRequest->floor == floor && currentRequest->orderType == orderType){
      return;
    }
  }

  int i = 0;
  currentRequest = baseRequest;
  while (currentRequest->child != NULL) {
    i++;
    currentRequest = currentRequest->child;
    bool elevatorGoingUp = state->direction_up;
    bool requestAboveCurrentGoal =
        baseRequest->child->floor < currentRequest->floor;

    if ((elevatorGoingUp && !requestAboveCurrentGoal &&
         (currentRequest->orderType != HARDWARE_ORDER_DOWN) && !(state->current_floor >= floor)) ||
        (!elevatorGoingUp && requestAboveCurrentGoal &&
         (currentRequest->orderType != HARDWARE_ORDER_UP) && !(state->current_floor <= floor))) {
      Request *newRequest = (Request *)malloc(sizeof(Request));
      _new_request(newRequest, currentRequest->parent, floor, orderType);
      // printf("Inserted %d\n", i);
      // printf("Queue length: %d \n", queueLength(baseRequest));
      return;
    }
  }

  // If at end of queue, insert
  if (currentRequest->child == NULL) {
    Request *newRequest = (Request *)malloc(sizeof(Request));
    _new_request(newRequest, currentRequest, floor, orderType);
    // printf("Inserted end\n");
    // printf("Queue length: %d \n", queueLength(baseRequest));
  }
  _dumpQueue(baseRequest);
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