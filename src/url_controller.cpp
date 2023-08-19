//
// Created by liuke on 2023/8/19.
//

#include "url_controller.h"
#include "stepper_controller.h"
#include "i2c_controller.h"
#include "vector"

void init_server(AsyncWebServer &s, std::string &_url_handle, AsyncWebServerRequest **_global_req) {
    std::vector<std::string> routes;
    routes.push_back("/");
    routes.push_back("/ctrl");
    routes.push_back("/move");
    routes.push_back("/scan");


    for (const auto &item: routes) {
        s.on(item.c_str(), HTTP_GET, [=, &_url_handle](AsyncWebServerRequest *request) mutable {
            // 好像只能用可变的值捕获
            _url_handle = item;
            *_global_req = request;
        });
    }

    s.begin();
}

void handle_url(AsyncWebServerRequest *_req, std::string &_url_handle, DynamicJsonDocument &doc, Stepper &stepper) {
    doc.garbageCollect();

    if (_url_handle.empty())return;
    else if (_url_handle == "/") {
        _req->send(SPIFFS, "/index.html");

    } else if (_url_handle == "/ctrl") {
        AsyncWebParameter *p = _req->getParam("value");
        auto res = _req->beginResponseStream("application/json");

        if (p->value() == "1") {
            digitalWrite(CTRL, 1);
        } else {
            digitalWrite(CTRL, 0);
        }
        doc["state"] = "success";
        serializeJson(doc, *res);
        _req->send(res);
    } else if (_url_handle == "/move") {
        AsyncWebParameter *p = _req->getParam("value");
        auto res = _req->beginResponseStream("application/json");
        int steps = p->value().toInt();

        doc["state"] = "success";
        serializeJson(doc, *res);
        _req->send(res);

        move_test(stepper, steps);

    } else if (_url_handle == "/scan") {
        _req->client()->setRxTimeout(5);
        auto res = _req->beginResponseStream("application/json");

        doc["state"] = "success";

        auto _map = devices_scan();
        auto data = doc.to<JsonArray>();
        for (const auto &item: _map) {
            data.add(item.first);
        }
        doc["data"] = data;

        serializeJson(doc, *res);
        _req->send(res);
    }

    _url_handle = "";

}


