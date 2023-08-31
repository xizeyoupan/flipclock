//
// Created by liuke on 2023/8/29.
//

#ifndef FLIPCLOCK_CONTENTPROVIDER_H
#define FLIPCLOCK_CONTENTPROVIDER_H

#include "config.h"

void init_external_peripheral();

std::map<int, int> displayTime();

std::map<int, int> displayRandom();

std::map<int, int> displayTemperature();

std::map<int, int> getIndex(const std::map<int, std::string> &s);

#endif //FLIPCLOCK_CONTENTPROVIDER_H
