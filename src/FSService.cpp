//
// Created by liuke on 2023/8/19.
//

#include "FSService.h"
#include "sstream"
#include <string>
#include "map"
#include "DeviceService.h"

extern std::map<int, int> addr_map;
extern std::map<int, std::pair<int, int>> zero_pos; // addr, index, offset
extern std::vector<std::vector<std::string>> contents;
extern std::map<std::string, std::map<int, int>> phrase;
extern std::vector<std::pair<std::string, int>> seq;

std::string format_config() {
    // format and read config from mem

    std::string output("contents\n");

    output.append(std::to_string(contents.size()));
    output.append("\n");
    for (const auto &group: contents) {
        for (const auto &item: group) {
            output.append(item + '\n');
        }
    }

    output.append("addr_map\n");
    output.append(std::to_string(addr_map.size()));
    output.append("\n");
    for (const auto &item: addr_map) {
        output.append(std::to_string(item.first));
        output.append(" ");
        output.append(std::to_string(item.second));
        output.append("\n");
    }

    output.append("zero_pos\n");
    output.append(std::to_string(zero_pos.size()));
    output.append("\n");
    for (const auto &item: zero_pos) {
        output.append(std::to_string(item.first));
        output.append(" ");
        output.append(std::to_string(item.second.first));
        output.append(" ");
        output.append(std::to_string(item.second.second));
        output.append("\n");
    }

    output.append("phrase\n");
    output.append(std::to_string(phrase.size()));
    output.append("\n");
    for (const auto &item: phrase) {
        output.append(item.first);
        output.append(" ");
        output.append(std::to_string(item.second.size()));
        output.append("\n");

        for (const auto &p: item.second) {
            output.append(std::to_string(p.first));
            output.push_back(' ');
            output.append(std::to_string(p.second));
            output.push_back('\n');
        }
    }

    output.append("seq\n");
    output.append(std::to_string(seq.size()));
    output.append("\n");
    for (const auto &item: seq) {
        output.append(item.first);
        output.append(" ");
        output.append(std::to_string(item.second));
        output.append("\n");
    }

    output.append("-1\n");
    return output;
}

void read_config(const char *config_path, std::string &original_data) {
    // parse and load config from file

    Serial.println("reading config...");
    auto config_file = SPIFFS.open(config_path);
    Serial.println("config opened.");

    if (!config_file) {
        Serial.println("There was an error opening the config file for reading");
        delay(2);
        return;
    }

    std::string s;
    while (config_file.available()) {
        s += config_file.read();
    }

    if (s.empty())return;

    original_data = s;

    std::istringstream iss(s);
    std::string op;
    while (!iss.eof()) {
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
                    one_content.push_back(info + " " + content);
//                    Serial.println(info.c_str());
                }
                contents.push_back(one_content);
            }
        }

        if (op == "addr_map") {

            int num;
            iss >> num;
            while (num--) {
                int addr, index;
                iss >> addr >> index;
                addr_map[addr] = index;
            }
        }

        if (op == "zero_pos") {
            int num;
            iss >> num;
            while (num--) {
                int addr, index;
                int offset;
                iss >> addr >> index >> offset;
                zero_pos[addr] = std::make_pair(index, offset);
            }
        }

        if (op == "phrase") {
            int num;
            iss >> num;
            while (num--) {
                int i, size, addr;
                std::string _phrase;
                std::map<int, int> index;
                iss >> _phrase >> size;
                for (int j = 0; j < size; ++j) {
                    iss >> addr >> i;
                    index[addr] = i;
                }

                phrase[_phrase] = index;
            }
        }

        if (op == "seq") {
            seq.clear();
            int num;
            iss >> num;
            while (num--) {
                std::string _phrase;
                int duration;
                iss >> _phrase >> duration;
                seq.emplace_back(_phrase, duration);
            }
        }
    }

    config_file.close();
}


void write_config(const char *config_path, const char *config) {

    auto config_file = SPIFFS.open(config_path, FILE_WRITE);
    if (!config_file) {
        Serial.println("There was an error opening the config file for writing");
        return;
    }

    if (config_file.print(config)) {
        Serial.println("Config file was written");;
    } else {
        Serial.println("Config file write failed");
    }

    config_file.close();
}