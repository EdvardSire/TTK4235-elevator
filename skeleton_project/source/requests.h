
#ifndef REQUESTS_H
#define REQUESTS_H
#include "hardware.h"
#include <stdbool.h>

typedef struct {
    int floor; // -1 indicates no request
    HardwareOrder orderType;
    struct request* child;
    struct request* parent;
} Request;

extern Request base_req;

void insert_request_last(int floor,  HardwareOrder orderType);
void purge_requests();

#endif