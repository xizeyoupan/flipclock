//
// Created by liuke on 2023/8/19.
//

#ifndef FLIPCLOCK_DEVICESERVICE_H
#define FLIPCLOCK_DEVICESERVICE_H

#include "config.h"
#include "map"

std::map<int, int> scan_device_and_content_provider();

int move_step(int step);

int set_switch(int addr, int state);

int set_switch_from_status();

int reset_pos(int addr);

int not_on_mag(int addr);

int release_motor();

int move_offset(int addr);

int get_current_index(int addr);

int move_to_flip(const std::map<int, int> &target);

int move_to_zero_pos_all();

void display_seq();

#endif //FLIPCLOCK_DEVICESERVICE_H
