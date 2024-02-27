#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include "hardware.h"
#include "requests.h"

Request base_req  = {.child=NULL, .parent=NULL};

void insert_request_last(int floor, HardwareOrder orderType, Request* base_req){
    Request* current_request = base_req;
    while(current_request->child != NULL ){
        current_request = current_request->child;
    };
    
    
    Request* new_req  = (Request *)malloc(sizeof(Request));
    if (new_req =! NULL){
        new_req->floor = floor;
        new_req->orderType = orderType;
        new_req -> parent = current_request;
        new_req -> child = NULL;
        current_request->child = new_req;
    }
};

void purge_reqests(){
    Request* current_request = &base_req;
    while(current_request->child != NULL ){
        current_request = (*current_request).child;
    };
    current_request = (*current_request).parent;
    while(current_request->parent != NULL ){
       free(current_request->child);
       current_request = current_request->parent;
    }
};
