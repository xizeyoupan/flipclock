//
// Created by liuke on 2023/8/19.
//

#ifndef FLIPCLOCK_URL_CONTROLLER_H
#define FLIPCLOCK_URL_CONTROLLER_H

#include "config.h"
#include "map"
#include "ArduinoJson.h"
void init_server(AsyncWebServer &s, std::string &_url_handle, AsyncWebServerRequest **_global_req);
void handle_url(AsyncWebServerRequest *_req, std::string &_url_handle, DynamicJsonDocument &doc, Stepper &stepper);


#endif //FLIPCLOCK_URL_CONTROLLER_H