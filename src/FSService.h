//
// Created by liuke on 2023/8/19.
//

#ifndef FLIPCLOCK_FSSERVICE_H
#define FLIPCLOCK_FSSERVICE_H

#include "config.h"
#include "map"

std::string format_config();

void read_config(const char *config_path, std::string &original_data);

void write_config(const char *config_path, const char *config);

#endif //FLIPCLOCK_FS_CONTROLLER_H
