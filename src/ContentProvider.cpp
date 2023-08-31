//
// Created by liuke on 2023/8/29.
//

#include "ContentProvider.h"
#include "Adafruit_Sensor.h"
#include "DHT.h"

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

extern std::vector<std::vector<std::string>> contents;
extern std::map<int, int> addr_map;
extern std::map<std::string, std::map<int, int>> phrase;

std::map<std::string, std::map<int, int>(*)()> providers = {
        {"显示时间", displayTime},
        {"显示温度", displayTemperature},
        {"随机显示", displayRandom},
};

void init_external_peripheral() {
    configTime(3600 * 8, 0, "ntp.ntsc.ac.cn", "pool.ntp.org");
    dht.begin();
}

std::map<int, int> displayRandom() {
    auto it = phrase.cbegin();

    while (true) {
        srand(millis());
        int dice = rand() % phrase.size();
        it = phrase.cbegin();
        while (dice--)it++;
        if (providers.find(it->first) == providers.end())break;
    }

    return it->second;
}

std::map<int, int> displayTime() {
    std::map<int, std::string> _time;

    struct tm time_info{};
    while (!getLocalTime(&time_info)) {
        Serial.println("Failed to obtain time");
    }

    char hour[3], minute[3];

    strftime(hour, 3, "%H", &time_info);
    strftime(minute, 3, "%M", &time_info);

    std::vector<std::string> temp{std::string(1, hour[0]), std::string(1, hour[1]), std::string(1, minute[0]),
                                  std::string(1, minute[1])};

    for (const auto &item: addr_map) {
        _time[item.first] = temp[item.second];
    }

    return getIndex(_time);
}

std::map<int, int> displayTemperature() {
    std::map<int, std::string> temperature;

    int t = dht.readTemperature();

    int _0, _1;
    _1 = t % 10;
    t /= 10;
    _0 = t % 10;

    std::vector<std::string> temp{"气", "温", std::to_string(_0), std::to_string(_1)};

    for (const auto &item: addr_map) {
        temperature[item.first] = temp[item.second];
    }

    return getIndex(temperature);
}

std::map<int, int> getIndex(const std::map<int, std::string> &s) {
    std::map<int, int> index;

    int y = 0;
    for (const auto &item: s) {

        for (int j = 0; j < contents[y].size(); ++j) {
            int k = 0;
            while (contents[y][j][k++] != ' ');
            int n = contents[y][j].size() - k;
            auto content = contents[y][j].substr(k, n);

//            Serial.println("s:");
//            Serial.println(item.second.c_str());
//            Serial.println("contents:");
//            Serial.println(contents[y][j].c_str());
//            Serial.println("k:");
//            Serial.println(k);
//            Serial.println("n:");
//            Serial.println(n);

            if (content == item.second) {
                index[item.first] = j;
//                Serial.println(j);
                break;
            }
        }

        y++;

    }

    return index;
}