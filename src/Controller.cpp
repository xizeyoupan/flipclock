//
// Created by liuke on 2023/8/19.
//

#include "Controller.h"
#include "FSService.h"
#include "DeviceService.h"
#include "map"

extern Stepper stepper;
extern AsyncWebServerRequest *global_request;
extern std::string url_handle;
extern std::string post_data;
extern bool post_data_end;
extern std::map<int, int> addr_map;
extern std::map<int, int> zero_pos;
extern std::vector<std::vector<std::string>> contents;

void init_server(AsyncWebServer &s) {

    std::map<std::string, std::string> routes;
    routes.insert({"/", ""});
    routes.insert({"/ctrl", ""});
    routes.insert({"/move", ""});
    routes.insert({"/scan", ""});
    routes.insert({"/upload_config", "json"});
    routes.insert({"/read_config", ""});
    routes.insert({"/adjust", ""});


    for (const auto &item: routes) {
        if (item.second == "") {
            s.on(
                    item.first.c_str(),
                    HTTP_GET,
                    [=](AsyncWebServerRequest *request) {
                        url_handle = item.first;
                        global_request = request;
                    }
            );

        } else if (item.second == "json") {

            s.on(
                    item.first.c_str(),
                    HTTP_POST,
                    [](AsyncWebServerRequest *request) {},
                    NULL,
                    [=]
                            (AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

                        if (!index) {
                            url_handle = item.first;
                            global_request = request;
                            post_data = "";
                            post_data_end = false;
                        }

                        for (size_t i = 0; i < len; i++) {
                            post_data += data[i];
                        }
                        if (index + len == total) {
                            post_data_end = true;
                        }
                    }
            );
        }

    }

    s.begin();
}

void handle_url(
        DynamicJsonDocument &doc
) {

    doc.garbageCollect();

    if (url_handle.empty())return;

    Serial.print("Request url: ");
    Serial.println(url_handle.c_str());

    if (url_handle == "/") {
        global_request->send(SPIFFS, "/index.html");
        url_handle = "";
        return;
    }

    auto res = global_request->beginResponseStream("application/json");

    if (url_handle == "/ctrl") {
        AsyncWebParameter *p = global_request->getParam("value");

        if (p->value() == "1") {
            digitalWrite(CTRL, 1);
        } else {
            digitalWrite(CTRL, 0);
        }
        doc["state"] = "success";
        doc["data"] = "";

    } else if (url_handle == "/move") {
        AsyncWebParameter *p = global_request->getParam("value");
        int steps = p->value().toInt();

//        move_test(stepper, steps);
        doc["state"] = "not imp";
        doc["data"] = "";


    } else if (url_handle == "/scan") {

        addr_map = scan_devices();
        doc["state"] = "success";
        JsonArray data = doc.createNestedArray("data");
        for (const auto &item: addr_map) {
            JsonObject each = data.createNestedObject();
            each["addr"] = item.first;
            each["index"] = item.second;
        }


    } else if (url_handle == "/upload_config") {

        while (!post_data_end)delay(1);

        auto err = deserializeJson(doc, post_data.c_str());
        if (err) {
            Serial.print("deserializeJson() failed with code ");
            Serial.println(err.c_str());
        }
        const char *data = doc["data"];
        write_config("/config", data);

        std::string original_data;
        if (addr_map.empty())addr_map = scan_devices();
        read_config("/config", original_data);
        auto output = format_config();

        doc["state"] = "success";
        doc["data"] = output;

    } else if (url_handle == "/read_config") {
        std::string original_data;

        if (addr_map.empty())addr_map = scan_devices();
        read_config("/config", original_data);
        doc["data"] = format_config();
        doc["state"] = "success";

    } else if (url_handle == "/adjust") {
        AsyncWebParameter *p = global_request->getParam("addr");
        int addr = p->value().toInt();

        doc["state"] = "success";
        doc["data"] = "";
        serializeJson(doc, *res);
        global_request->send(res);
        url_handle = "";
        reset_pos(addr);
        return;
    }


    serializeJson(doc, *res);
    global_request->send(res);
    url_handle = "";

}
