//
// Created by liuke on 2023/8/19.
//

#include "fs_controller.h"
#include "sstream"

void read_config(std::string &config_path,
                 std::vector<std::vector<std::string>> &contents) {

    auto config_file = SPIFFS.open(config_path.c_str());

    if (!config_file) {
        Serial.println("There was an error opening the config file for reading");
        return;
    }


    std::string s;
    while (config_file.available()) {
        s += config_file.read();
    }

    std::istringstream iss(s);
    std::string op;
    while (true) {
        iss >> op;
        if (op == "-1")break;

        if (op == "contents") {
            contents.clear();
            int num;
            iss >> num;
            while (num--) {
                std::vector<std::string> one_content;
                for (int i = 0; i < 40; ++i) {
                    std::string info, content;
                    iss >> info >> content;
                    one_content.push_back(info);
                }
                contents.push_back(one_content);
            }
        }
    }

    config_file.close();
}


void write_config(std::string &config_path, std::string &config) {

    auto config_file = SPIFFS.open(config_path.c_str(), FILE_WRITE);
    if (!config_file) {
        Serial.println("There was an error opening the config file for writing");
        return;
    }

    if (config_file.print("TEST")) {
        Serial.println("Config file was written");;
    } else {
        Serial.println("Config file write failed");
    }

    config_file.close();
}