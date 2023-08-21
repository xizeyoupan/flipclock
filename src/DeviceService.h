//
// Created by liuke on 2023/8/19.
//

#ifndef FLIPCLOCK_DEVICESERVICE_H
#define FLIPCLOCK_DEVICESERVICE_H

#include "config.h"
#include "map"

std::map<int, int> scan_devices();
void move_test(Stepper &stepper, int move);

#endif //FLIPCLOCK_DEVICESERVICE_H
