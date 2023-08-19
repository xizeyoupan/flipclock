//
// Created by liuke on 2023/8/19.
//

#ifndef FLIPCLOCK_FS_CONTROLLER_H
#define FLIPCLOCK_FS_CONTROLLER_H

#include "config.h"

void read_config(std::string &config_path,
                 std::vector<std::vector<std::string>> &contents);

void write_config(std::string &config_path, std::string &config);

#endif //FLIPCLOCK_FS_CONTROLLER_H
