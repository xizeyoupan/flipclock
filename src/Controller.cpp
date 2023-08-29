//
// Created by liuke on 2023/8/19.
//

#include "Controller.h"
#include "FSService.h"
#include "DeviceService.h"
#include "map"

DynamicJsonDocument doc(16 * 1024);
AsyncWebServer server(80);

AsyncWebServerRequest *global_request;
std::string url_handle;
std::string post_data;
bool post_data_end;

std::map<int, int> addr_map;
std::map<int, std::pair<int, double>> zero_pos; // addr, index, offset
std::vector<std::vector<std::string>> contents;
std::map<int, int> flip_pos;
std::map<int, double> flip_degree;
std::map<std::string, std::vector<int>> phrase;


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
    routes.insert({"/set_phrase", ""});


    for (const auto &item: routes) {
        if (item.second == "") {
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

    } else if (url_handle == "/save_config") {

        if (addr_map.empty())addr_map = scan_devices();

        std::string config = format_config();
        write_config("/config", config.c_str());

        doc["state"] = "success";
        doc["data"] = config;

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
    } else if (url_handle == "/set_phrase") {
        AsyncWebParameter *a = global_request->getParam("action");
        AsyncWebParameter *p = global_request->getParam("phrase");
        String action = a->value();
        String _phrase = p->value();
        String seq;
        if (action.equals("add")) {
            AsyncWebParameter *s = global_request->getParam("seq");
            seq = s->value();
            std::string _temp;
            std::vector<std::string> strseq;
            for (const auto &item: seq) {
                if (item != ',')_temp += item;
                else {
                    if (!_temp.empty()) {
                        strseq.push_back(_temp);
                        _temp = "";
                    }
                }
            }
            if (!_temp.empty()) {
                strseq.push_back(_temp);
                _temp = "";
            }

            std::vector<int> index_seq;
            for (const auto &item: strseq) {
                index_seq.push_back(std::stoi(item));
            }
            phrase[_phrase.c_str()] = index_seq;

        } else if (action.equals("remove")) {
            phrase.erase(_phrase.c_str());
        }

        std::string config = format_config();
        write_config("/config", config.c_str());

        doc["state"] = "success";
        doc["data"] = "";

    } else if (url_handle == "/set_zero_pos") {
        AsyncWebParameter *a = global_request->getParam("addr");
        AsyncWebParameter *i = global_request->getParam("index");
        int addr = a->value().toInt();
        int index = i->value().toInt();

        zero_pos[addr].first = index;

        std::string output = format_config();
        write_config("/config", output.c_str());

        doc["state"] = "success";
        doc["data"] = "";

    }


    serializeJson(doc, *res);
    global_request->send(res);
    url_handle = "";

}
