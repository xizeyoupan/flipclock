//
// Created by liuke on 2023/8/19.
//

#ifndef FLIPCLOCK_CONTROLLER_H
#define FLIPCLOCK_CONTROLLER_H

#include "config.h"
#include "map"
#include "ArduinoJson.h"

void init_server(AsyncWebServer &s);

void handle_url(
        DynamicJsonDocument &doc
);

#endif //FLIPCLOCK_CONTROLLER_H
