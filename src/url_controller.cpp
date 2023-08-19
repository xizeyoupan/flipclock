//
// Created by liuke on 2023/8/19.
//

#include "url_controller.h"
#include "stepper_controller.h"
#include "i2c_controller.h"
#include "map"

void init_server
        (AsyncWebServer &s,
         std::string &_url_handle,
         AsyncWebServerRequest **_global_req,
         std::string &_post_data,
         bool &_post_data_end) {

    std::map<std::string, std::string> routes;
    routes.insert({"/", ""});
    routes.insert({"/ctrl", ""});
    routes.insert({"/move", ""});
    routes.insert({"/scan", ""});
    routes.insert({"/upload_config", "json"});


    for (const auto &item: routes) {
        if (item.second == "") {

            s.on(
                    item.first.c_str(),
                    HTTP_GET,
                    [=, &_url_handle](AsyncWebServerRequest *request) mutable {
                        // _global_req好像只能用可变的值捕获
                        _url_handle = item.first;
                        *_global_req = request;
                    }
            );

        } else if (item.second == "json") {

            s.on(
                    item.first.c_str(),
                    HTTP_POST,
                    [](AsyncWebServerRequest *request) {},
                    NULL,
                    [=, &_url_handle, &_post_data, &_post_data_end]
                            (AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
                             size_t total) mutable {

                        if (!index) {
                            _url_handle = item.first;
                            *_global_req = request;
//                            Serial.printf("BodyStart: %u B\n", total);
                            _post_data = "";
                            _post_data_end = false;
                        }

                        for (size_t i = 0; i < len; i++) {
//                            Serial.write(data[i]);
                            _post_data += data[i];
                        }
                        if (index + len == total) {
//                            Serial.printf("BodyEnd: %u B\n", total);
                            _post_data_end = true;
                        }
                    }
            );
        }

    }

    s.begin();
}

void handle_url
        (AsyncWebServerRequest *_req,
         std::string &_url_handle,
         DynamicJsonDocument &doc,
         Stepper &stepper,
         std::string &_post_data,
         bool &_post_data_end) {

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
    } else if (_url_handle == "/upload_config") {
        _req->client()->setRxTimeout(5);
        auto res = _req->beginResponseStream("application/json");

        while (!_post_data_end)delay(1);

        Serial.println(_post_data.c_str());

        doc["state"] = "success";
        serializeJson(doc, *res);
        _req->send(res);
    }

    _url_handle = "";

}


