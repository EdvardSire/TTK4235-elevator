#include "requests.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

void _dumpQueue(Request baseRequest[static 1]) {
  Request *current = baseRequest;
  printf("Queue after insertion: ");
  while (current->child != NULL) {
    printf("%d ", current->child->floor);
    current = current->child;
  }
  printf("\n");
}

void handleRequestLast(int floor, Request baseRequest[static 1]) {
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

// 1.
void handleRequest(State FSM[static 1], Request baseRequest[static 1],
                   int floorRequest, HardwareOrder orderType) {
  if (baseRequest->child == NULL) { // No requests in queue
    _insertRequest(baseRequest, NULL, floorRequest);
    _dumpQueue(baseRequest);
  } else { // Existings requests

    // Get direction
    int current_direction;
    if (baseRequest->child->floor > FSM->current_floor)
      current_direction = true; // up
    else
      current_direction = false; // down

    int order_direction;
    if (orderType == HARDWARE_ORDER_UP)
      order_direction = true;
    else
      order_direction = false;

    // Inside orders
    if (orderType == HARDWARE_ORDER_INSIDE) {
      // Matches on the up
      if ((FSM->current_floor < floorRequest) &&
          (floorRequest < baseRequest->child->floor)) {
        _insertRequest(baseRequest, baseRequest->child, floorRequest);
        _dumpQueue(baseRequest);
        return;
      }
      // Mathces on the down
      if ((FSM->current_floor > floorRequest) &&
          (floorRequest > baseRequest->child->floor)) {
        _insertRequest(baseRequest, baseRequest->child, floorRequest);
        _dumpQueue(baseRequest);
        return;
      }
      // Check child child
      if (baseRequest->child->child != NULL) {
        // Matches on the up
        if ((FSM->current_floor < floorRequest) &&
            (floorRequest < baseRequest->child->child->floor)) {
          _insertRequest(baseRequest, baseRequest->child, floorRequest);
          _dumpQueue(baseRequest);
          return;
        }
        // Mathces on the down
        if ((FSM->current_floor > floorRequest) &&
            (floorRequest > baseRequest->child->child->floor)) {
          _insertRequest(baseRequest, baseRequest->child, floorRequest);
          _dumpQueue(baseRequest);
          return;
        }
      }

    } else if ((current_direction == order_direction)) { // Hall up or down orders
      // Matches on the up
      if ((FSM->current_floor < floorRequest) &&
          (floorRequest < baseRequest->child->floor)) {
        _insertRequest(baseRequest, baseRequest->child, floorRequest);
        _dumpQueue(baseRequest);
        return;
      }
      // Mathces on the down
      if ((FSM->current_floor > floorRequest) &&
          (floorRequest > baseRequest->child->floor)) {
        _insertRequest(baseRequest, baseRequest->child, floorRequest);
        _dumpQueue(baseRequest);
        return;
      }
      // Check child child
      if (baseRequest->child->child != NULL) {
        // Matches on the up
        if ((FSM->current_floor < floorRequest) &&
            (floorRequest < baseRequest->child->child->floor)) {
          _insertRequest(baseRequest, baseRequest->child, floorRequest);
          _dumpQueue(baseRequest);
          return;
        }
        // Mathces on the down
        if ((FSM->current_floor > floorRequest) &&
            (floorRequest > baseRequest->child->child->floor)) {
          _insertRequest(baseRequest, baseRequest->child, floorRequest);
          _dumpQueue(baseRequest);
          return;
        }
      }
    }

    // Nothing happened
    Request *current = baseRequest;
    while (current->child != NULL)
      current = current->child;
    _insertRequest(current, NULL, floorRequest);
    _dumpQueue(baseRequest);
  }

  // int requestInserted = false;
  // Request *current = baseRequest;
  // while (current->child != NULL) {
  //   // Matches on the down
  //   if ((floorRequest > current->child->floor) &&
  //       (floorRequest < FSM->current_floor)) {
  //     _insertRequest(current, current->child, floorRequest);
  //     requestInserted = true;
  //     break;
  //   }
  //   // Matches on the up
  //   if ((floorRequest < current->child->floor) &&
  //       (floorRequest > FSM->current_floor)) {
  //     _insertRequest(current, current->child, floorRequest);
  //     requestInserted = true;
  //     break;
  //   }
  //   current = current->child;
  // }

  // if (!requestInserted)
  //   _insertRequest(current, NULL, floorRequest);
}

void deleteAllRequest(Request baseRequest[static 1]) {
  while (baseRequest->child != NULL)
    removeRequest(baseRequest->child);
}

void removeRequest(Request request[static 1]) {
  if (request->child == NULL) {
    request->parent->child = NULL;
    free(request);
  } else {
    request->parent->child = request->child;
    request->child->parent = request->parent;
    free(request);
  }
}