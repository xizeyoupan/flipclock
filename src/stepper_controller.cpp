//
// Created by liuke on 2023/8/19.
//

#include "stepper_controller.h"


void move_test(Stepper &stepper, int move) {
    stepper.step(stepsPerRevolution / move);
}