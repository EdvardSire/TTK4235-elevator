#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include "hardware.h"

static void clear_all_order_lights(){
    HardwareOrder order_types[3] = {
        HARDWARE_ORDER_UP,
        HARDWARE_ORDER_INSIDE,
        HARDWARE_ORDER_DOWN
    };

    for(int f = 0; f < HARDWARE_NUMBER_OF_FLOORS; f++){
        for(int i = 0; i < 3; i++){
            HardwareOrder type = order_types[i];
            hardware_command_order_light(f, type, 0);
        }
    }
}

static void lights() {
        /* All buttons must be polled, like this: */
        for(int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++){
            if(hardware_read_floor_sensor(floor)){
                hardware_command_floor_indicator_on(floor);
            }
        }

        /* Lights are set and cleared like this: */
        for(int floor = 0; floor < HARDWARE_NUMBER_OF_FLOORS; floor++){
            /* Internal orders */
            if(hardware_read_order(floor, HARDWARE_ORDER_INSIDE)){
                hardware_command_order_light(floor, HARDWARE_ORDER_INSIDE, 1);
            }

            /* Orders going up */
            if(hardware_read_order(floor, HARDWARE_ORDER_UP)){
                hardware_command_order_light(floor, HARDWARE_ORDER_UP, 1);
            }

            /* Orders going down */
            if(hardware_read_order(floor, HARDWARE_ORDER_DOWN)){
                hardware_command_order_light(floor, HARDWARE_ORDER_DOWN, 1);
            }
        }
}


int main(){
    if(hardware_init() != 0){
        fprintf(stderr, "Unable to initialize hardware\n");
        exit(1);
    }
    printf("Press the stop button on the elevator panel to exit\n");


    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);

    while(true){

      if(hardware_read_floor_sensor(0))
        hardware_command_movement(HARDWARE_MOVEMENT_UP);

      if(hardware_read_stop_signal())
        printf("hehe");

      // if(hardware_read_floor_sensor(HARDWARE_NUMBER_OF_FLOORS-2))
      //   hardware_command_movement(HARDWARE_MOVEMENT_STOP);

      lights();
    }

    return 0;
}
