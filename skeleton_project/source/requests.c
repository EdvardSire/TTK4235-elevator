// DUM
#include "requests.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void insert_request_last(int floor, HardwareOrder orderType,
                         Request baseRequest[static 1]) {
  Request *current_request = baseRequest;
  // Get bottom request
  while (current_request->child != NULL)
    current_request = current_request->child;

  // Create the new one
  Request *new = (Request *)malloc(sizeof(Request));
  new->parent = current_request;
  new->child = NULL;
  new->floor = floor;

  // Update the bottom one
  current_request->child = new;

  // Debug
  _dumpQueue(baseRequest);
};

void handleRequest(int floorRequest, Request baseRequest[static 1],
                   State FSM[static 1]) {
  // No requests
  if (baseRequest->child == NULL) {
    _insertRequest(baseRequest, NULL, floorRequest);
  } else { // Existings requests

    // TODO JUST PRUNE THE REQUEST
    int requestInserted = false;
    Request *current = baseRequest;
    while (current->child != NULL) {
      // Matches on the down
      if((floorRequest > current->child->floor) && (floorRequest < FSM->current_floor)) {
        _insertRequest(current, current->child, floorRequest);
        requestInserted = true;
        break;
      }
      // Matches on the up
      if((floorRequest < current->child->floor) && (floorRequest > FSM->current_floor)) {
        _insertRequest(current, current->child, floorRequest);
        requestInserted = true;
        break;
      }
      current = current->child;
    }

    if(!requestInserted)
      _insertRequest(current, NULL, floorRequest);

  }

  _dumpQueue(baseRequest);
}

void _insertRequest(Request *parent, Request *child, int floor) {
  Request *new = (Request *)malloc(sizeof(Request));
  // Handle new
  new->floor = floor;
  new->parent = parent;
  new->child = child;

  // Update others
  parent->child = new;
  if (child != NULL)
    child->parent = new;
}

///

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

void _dumpQueue(Request baseRequest[static 1]) {
  Request *current = baseRequest;
  while (current->child != NULL) {
    printf("%d ", current->child->floor);
    current = current->child;
  }
  printf("\n");
}

void _free_request(Request request[static 1]) {
  Request *parent = request->parent;
  if (request->child == NULL) { // At bottom
    parent->child = NULL;
  } else {
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
  newRequest->parent = parent;
  newRequest->child = parent->child; // important this first
  parent->child = newRequest;
  // printf("New request, floor: %d \n", floor);
}