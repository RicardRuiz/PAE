/*
 * lib_controllers.c
 *
 *  Created on: 2 d’abr. 2019
 *      Author: mat.aules
 */

#include "lib_controllers.h"
/*
uint16_t current_speed = 0; // the set speed for the robot. Even if the robot is stopped, this speed will indicate the
                            // speed the robot will take when it is turned on
uint8_t mov_state = 0; // State of the robot movement. This state is necessary to be able of changing the speed without changing the type of movement
*/
/**********************************************************+
        States to manage the type of movements:
        Move forward, mov_state = 1
        Turn right, mov_state = 2
        Turn left, mov_state = 3
        Move backward, mov_state = 4
        Stopped, mov_state = 5
        Spin right, mov_state = 6
        Spin left, mov_state = 7
***********************************************************/
/*
void move_forward(void) {
    // Parameters for moving forward (we change both the high and the low speed registers of both of the motors)
    uint8_t parameters[8] = {0x20, 0x02, 0x02, current_speed, (current_speed >> 8)|BIT2, 0x03, current_speed, current_speed >> 8};
    TxPacket(0xFE, 0x08, 0x83, parameters); // Send the parameters to the  unit
    mov_state = 1; // Change the state
}


void change_speed(uint16_t speed) {
    current_speed = speed; // Change the speed and continues with the current movement
    switch(mov_state) {
        case 1:
            move_forward();
            break;
        case 2:
            turn_right();
            break;
        case 3:
            turn_left();
            break;
        case 4:
            move_backward();
            break;
        case 6:
            spin_right();
            break;
        case 7:
            spin_left();
            break;
        default:
            stop_movement();
            break;
    }
}


void turn_right(void) {
    // Parameters for turning right (we change both the high and the low speed registers of both of the motors)
    uint8_t parameters[8] = {0x20, 0x02, 0x02, 0, 0, 0x03, current_speed, current_speed >> 8};
    TxPacket(0xFE, 0x08, 0x83, parameters); // Send the parameters to the  unit
    mov_state = 2; // Change the state
}


void turn_left(void) {
    // Parameters for turning left (we change both the high and the low speed registers of both of the motors)
    uint8_t parameters[8] = {0x20, 0x02, 0x02, current_speed, (current_speed >> 8)|BIT2, 0x03, 0, 0};
    TxPacket(0xFE, 0x08, 0x83, parameters); // Send the parameters to the  unit
    mov_state = 3; // Change the state
}


void move_backward(void) {
    // Parameters for moving backward (we change both the high and the low speed registers of both of the motors)
    uint8_t parameters[8] = {0x20, 0x02, 0x02, current_speed, current_speed >> 8, 0x03, current_speed, (current_speed >> 8)|BIT2};
    TxPacket(0xFE, 0x08, 0x83, parameters); // Send the parameters to the  unit
    mov_state = 4; // Change the state
}


void stop_movement(void) {
    // Parameters for stopping the movement (we change both the high and the low speed registers of both of the motors to 0)
    uint8_t parameters[8] = {0x20, 0x02, 0x02, 0x00, 0x00|BIT2, 0x03, 0x00, 0x00};
    TxPacket(0xFE, 0x08, 0x83, parameters); // Send the parameters to the  unit
    mov_state = 5; // Change the state
}

void switch_led(uint8_t id, uint8_t value) {
    // Parameters for the led (depending on the value it is turned on or off)
    uint8_t parameters[2] = {0x19, value};
    TxPacket(id, 0x02, 0x03, parameters); // Send the parameters to the unit

    struct RxReturn error = RxPacket();
    if (error.error) { // If an error is found on the returned status packet, then be turn off all the leds with broadcasting id
        uint8_t parameters[6] = {0x19, 0x01, 0x02, 0x00, 0x03, 0x00};
        TxPacket(0xFE, 0x06, 0x83, parameters);
    }
}

void spin_right(void) {
    // Parameters for spinning right (we change both the high and the low speed registers of both of the motors)
    uint8_t parameters[8] = {0x20, 0x02, 0x02, current_speed, current_speed >> 8, 0x03, current_speed, current_speed >> 8};
    TxPacket(0xFE, 0x08, 0x83, parameters); // Send the parameters to the  unit
    mov_state = 6; // Change the state
}

void spin_left(void) {
    // Parameters for spinning left (we change both the high and the low speed registers of both of the motors)
    uint8_t parameters[8] = {0x20, 0x02, 0x02, current_speed, (current_speed >> 8)|BIT2, 0x03, current_speed, (current_speed >> 8)|BIT2};
    TxPacket(0xFE, 0x08, 0x83, parameters); // Send the parameters to the  unit
    mov_state = 7; // Change the state
}
void abstract_movement(uint16_t speed[2], uint8_t direction[2]) {

    // Depending on the direction of each motor, the parameters will change
    if (!direction[0] && !direction[1]) {
        uint8_t parameters[8] = {0x20, 0x02, 0x02, speed[0], (speed[0] >> 8)|BIT2, 0x03, speed[1], speed[1] >> 8};
        TxPacket(0xFE, 0x08, 0x83, parameters);
    }
    else if (!direction[0] && direction[1]) {
        uint8_t parameters[8] = {0x20, 0x02, 0x02, speed[0], (speed[0] >> 8)|BIT2, 0x03, speed[1], (speed[1] >> 8)|BIT2};
        TxPacket(0xFE, 0x08, 0x83, parameters);
    }
    else if (direction[0] && !direction[1]) {
        uint8_t parameters[8] = {0x20, 0x02, 0x02, speed[0], speed[0] >> 8, 0x03, speed[1], speed[1] >> 8};
        TxPacket(0xFE, 0x08, 0x83, parameters);
    }
    else if (direction[0] && direction[1]){
        uint8_t parameters[8] = {0x20, 0x02, 0x02, speed[0], speed[0] >> 8, 0x03, speed[1], (speed[1] >> 8)|BIT2};
        TxPacket(0xFE, 0x08, 0x83, parameters);
    }
    else {
        stop_movement();
    }
}


uint8_t* getDistanceSensor(void) {
    // Parameters for the request of data
    uint8_t parameters[2] = {0x1A,0x03};
    TxPacket(0x04, 0x02, 0x02, parameters);
    static uint8_t r[3]; // Array to be returned with the information of the front, left and right directions

    struct RxReturn data = RxPacket();
    if (data.error) { // If the information is not correct, we return distance 0 in all cases (which is impossible)
        r[0] = 0;
        r[1] = 0;
        r[2] = 0;
    }
    else{
        r[0] = data.statusPacket[5];
        r[1] = data.statusPacket[6];
        r[2] = data.statusPacket[7];
    }

    return r;
}
*/
