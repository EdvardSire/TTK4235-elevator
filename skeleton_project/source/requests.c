// DUM
#include "requests.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

void handleRequest(int floorRequest, Request baseRequest[static 1],
                   State FSM[static 1]) {
  if (baseRequest->child == NULL) { // No requests in queue
    _insertRequest(baseRequest, NULL, floorRequest);
  } else { // Existings requests

    // TODO PRUNE THE REQUEST
    int requestInserted = false;
    Request *current = baseRequest;
    while (current->child != NULL) {
      // Matches on the down
      if ((floorRequest > current->child->floor) &&
          (floorRequest < FSM->current_floor)) {
        _insertRequest(current, current->child, floorRequest);
        requestInserted = true;
        break;
      }
      // Matches on the up
      if ((floorRequest < current->child->floor) &&
          (floorRequest > FSM->current_floor)) {
        _insertRequest(current, current->child, floorRequest);
        requestInserted = true;
        break;
      }
      current = current->child;
    }

    if (!requestInserted)
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

void _dumpQueue(Request baseRequest[static 1]) {
  Request *current = baseRequest;
  printf("Queue after insertion: ");
  while (current->child != NULL) {
    printf("%d ", current->child->floor);
    current = current->child;
  }
  printf("\n");
}