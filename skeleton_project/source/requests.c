#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include "hardware.h"
#include "requests.h"
#include "FSM.h"


void insert_request_last(int floor, HardwareOrder orderType, Request* base_req){
    Request* current_request = base_req;
    while(current_request->child != NULL )
        current_request = current_request->child;
    
    
    Request* new_req  = (Request *)malloc(sizeof(Request));
    if ((new_req != NULL)){
        new_req->floor = floor;
        new_req->orderType = orderType;
        new_req->parent = current_request;
        new_req->child = NULL;
        current_request->child = new_req;
    }
};

void insert_request(int floor, HardwareOrder orderType, Request* base_req, State* state){
    Request* current_request = base_req;


    while(current_request->child != NULL ){
        current_request = current_request->child;
        bool elevator_going_up = state->current_floor < state->going_to_floor;
        bool req_floor_above_current_destination = state->going_to_floor < current_request->floor;

        if( (elevator_going_up && !req_floor_above_current_destination && current_request->orderType!=HARDWARE_ORDER_DOWN)
            || 
            (!elevator_going_up && req_floor_above_current_destination && current_request->orderType!=HARDWARE_ORDER_UP)
            ){
            Request* new_req  = (Request *)malloc(sizeof(Request));
            if ((new_req != NULL)){
                new_req->floor = floor;
                new_req->orderType = orderType;
                new_req->parent = current_request->parent;
                new_req->child = current_request;
                current_request->parent = new_req;
            }
        }

        // If at end of queue, insert
        if (current_request->child == NULL){
            Request* new_req  = (Request *)malloc(sizeof(Request));
            if ((new_req != NULL)){
                new_req->floor = floor;
                new_req->orderType = orderType;
                new_req->parent = current_request;
                new_req->child = NULL;
                current_request->child = new_req;
        }
    }
}

void purge_requests(Request * base_req){
    Request* current_request = base_req;
    while(current_request->child != NULL )
        current_request = current_request->child;

    while(current_request->parent != NULL ){
       current_request = current_request->parent;
       free(current_request->child);
    }
    base_req->child = NULL;
}

void floor_request_filled(int floor, Request* base_req){
    Request* current_request = base_req;
    // Traverse LL to bottom
    while(current_request->child != NULL )
        current_request = current_request->child; 

    while(current_request->parent != NULL ){
        current_request = current_request->parent;
        if(current_request->child->floor == floor) 
            free(current_request->child);      
    }
}