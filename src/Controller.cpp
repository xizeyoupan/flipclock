//
// Created by liuke on 2023/8/19.
//

#include "Controller.h"
#include "FSService.h"
#include "DeviceService.h"
#include "ContentProvider.h"

DynamicJsonDocument doc(16 * 1024);
AsyncWebServer server(80);

AsyncWebServerRequest *global_request;
std::string url_handle;
std::string post_data;
bool post_data_end;

std::map<int, int> addr_map;
std::map<int, std::pair<int, double>> zero_pos; // addr, index, offset
std::vector<std::vector<std::string>> contents;

std::map<int, double> flip_degree;
std::map<std::string, std::map<int, int>> phrase; // phrase, addr, content index

extern std::map<std::string, std::vector<std::string>(*)()> providers;


void init_server() {

    std::map<std::string, std::string> routes;
    routes.insert({"/", ""});
//    routes.insert({"/favicon.ico", ""});
    routes.insert({"/ctrl", ""});
    routes.insert({"/move", ""});
    routes.insert({"/scan", ""});
    routes.insert({"/upload_config", "json"});
    routes.insert({"/read_config", ""});
    routes.insert({"/save_config", ""});
    routes.insert({"/adjust", ""});
    routes.insert({"/set_zero_pos", ""});
    routes.insert({"/set_phrase", "json"});
    routes.insert({"/test", ""});
    routes.insert({"/reboot", ""});


    for (const auto &item: routes) {
        if (item.second.empty()) {
            server.on(
                    item.first.c_str(),
                    HTTP_GET,
                    [=](AsyncWebServerRequest *request) {
                        url_handle = item.first;
                        global_request = request;
                    }
            );

        } else if (item.second == "json") {

            server.on(
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

    server.begin();
}

void handle_url() {

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
        AsyncWebParameter *d = global_request->getParam("degree");
        AsyncWebParameter *a = global_request->getParam("addr");

        double degrees = d->value().toDouble();
        int addr = a->value().toInt();

        set_switch(addr, HIGH);

        doc["state"] = "success";
        doc["data"] = "";

        move_degree(degrees);
        release_motor();

        zero_pos[addr].second = flip_degree[addr];


    } else if (url_handle == "/scan") {

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
        write_config(CONFIG_PATH, data);

        std::string original_data;

        read_config(CONFIG_PATH, original_data);
        auto output = format_config();

        doc["state"] = "success";
        doc["data"] = output;

    } else if (url_handle == "/save_config") {


        std::string config = format_config();
        write_config(CONFIG_PATH, config.c_str());

        doc["state"] = "success";
        doc["data"] = config;

    } else if (url_handle == "/read_config") {
        std::string original_data;

        read_config(CONFIG_PATH, original_data);
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
    } else if (url_handle == "/set_phrase") {
        while (!post_data_end)delay(1);

        auto err = deserializeJson(doc, post_data.c_str());
        if (err) {
            Serial.print("deserializeJson() failed with code ");
            Serial.println(err.c_str());
        }


        JsonObject data = doc["data"];

        const char *data_action = data["action"];
        const char *data_phrase = data["phrase"];


        if (strcmp(data_action, "add") == 0) {
            for (JsonObject item: data["seq"].as<JsonArray>()) {
                int addr = item["addr"];
                int index = item["index"];
                phrase[data_phrase][addr] = index;
            }

        } else if (strcmp(data_action, "remove") == 0) {
            phrase.erase(data_phrase);
        }

        std::string config = format_config();
        write_config(CONFIG_PATH, config.c_str());

        doc["state"] = "success";
        doc["data"] = "";

    } else if (url_handle == "/set_zero_pos") {
        AsyncWebParameter *a = global_request->getParam("addr");
        AsyncWebParameter *i = global_request->getParam("index");
        int addr = a->value().toInt();
        int index = i->value().toInt();

        zero_pos[addr].first = index;

        std::string output = format_config();
        write_config(CONFIG_PATH, output.c_str());

        doc["state"] = "success";
        doc["data"] = "";

    } else if (url_handle == "/test") {

        auto s = providers["显示时间"]();
        auto i = getIndex(s);

        for (const auto &item: i) {
            Serial.println(item);
        }
        doc["state"] = "success";
        doc["data"] = "";

    } else if (url_handle == "/reboot") {
        ESP.restart();
    }


    serializeJson(doc, *res);
    global_request->send(res);
    url_handle = "";

}
