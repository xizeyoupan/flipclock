//
// Created by liuke on 2023/8/19.
//

#ifndef FLIPCLOCK_DEVICESERVICE_H
#define FLIPCLOCK_DEVICESERVICE_H

#include "config.h"
#include "map"

std::map<int, int> scan_devices();

void move_degree(int degree);

int set_switch(int addr, int state);

int reset_pos(int addr);

int not_on_mag(int addr);

#endif //FLIPCLOCK_DEVICESERVICE_H
