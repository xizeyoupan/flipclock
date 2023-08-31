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
std::map<int, std::pair<int, int>> zero_pos; // addr, index, offset
std::vector<std::vector<std::string>> contents;

std::map<int, int> flip_pos;
std::map<std::string, std::map<int, int>> phrase; // phrase, addr, content index
std::map<int, int> switch_status;

std::vector<std::pair<std::string, int>> seq;

extern std::map<std::string, std::map<int, int>(*)()> providers;

extern int base_step;


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
    routes.insert({"/set_seq", "json"});
    routes.insert({"/set_base_step", ""});
    routes.insert({"/move_to_zero", ""});


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
                    nullptr,
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
        AsyncWebParameter *d = global_request->getParam("step");
        AsyncWebParameter *a = global_request->getParam("addr");

        int step = d->value().toInt();
        int addr = a->value().toInt();

        set_switch(addr, HIGH);

        zero_pos[addr].second += step;

        std::string config = format_config();
        write_config(CONFIG_PATH, config.c_str());

        doc["state"] = "success";
        doc["data"] = config;

        move_step(step);
        release_motor();

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
        doc["data"] = config;

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
        AsyncWebParameter *p = global_request->getParam("p");
        auto volatile s = p->value().c_str();

        doc["state"] = "success";
        doc["data"] = "";

        std::string ss(s);

        serializeJson(doc, *res);
        global_request->send(res);
        url_handle = "";

//        move_to_zero_pos_all();


        if (providers.find(ss) != providers.end()) {
            move_to_flip(providers[s]());
        } else if (phrase.find(ss) != phrase.end()) {
            move_to_flip(phrase[s]);
        } else {
            Serial.println(5565);

        }

        return;

    } else if (url_handle == "/set_seq") {
        while (!post_data_end)delay(1);

        auto err = deserializeJson(doc, post_data.c_str());
        if (err) {
            Serial.print("deserializeJson() failed with code ");
            Serial.println(err.c_str());
        }

        seq.clear();
        int from = doc["data"]["from"];

        for (JsonObject data_item: doc["data"]["seq"].as<JsonArray>()) {

            const char *_phrase = data_item["phrase"];
            int duration = data_item["duration"];

            if (phrase.find(_phrase) == phrase.end()) {
                Serial.println("Wrong phrase: ");
                Serial.println(_phrase);
                continue;
            }

            seq.emplace_back(_phrase, duration);

        }

        std::string output = format_config();
        write_config(CONFIG_PATH, output.c_str());

        doc["state"] = "success";
        doc["data"] = output;

    } else if (url_handle == "/move_to_zero") {

        doc["state"] = "success";
        doc["data"] = "";
        serializeJson(doc, *res);
        global_request->send(res);
        url_handle = "";

        move_to_zero_pos_all();

        return;
    } else if (url_handle == "/set_base_step") {
        AsyncWebParameter *s = global_request->getParam("s");
        base_step = s->value().toInt();
        doc["state"] = "success";
        doc["data"] = "";

    } else if (url_handle == "/reboot") {
        ESP.restart();
    }


    serializeJson(doc, *res);

    if (global_request->client() != nullptr)global_request->send(res);
    url_handle = "";

}
