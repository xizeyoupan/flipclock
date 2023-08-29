//
// Created by liuke on 2023/8/29.
//

#include "ContentProvider.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, 28800);

extern std::vector<std::vector<std::string>> contents;

std::map<std::string, std::vector<std::string>(*)()> providers = {{"显示时间", displayTime}};

std::vector<std::string> displayTime() {
    std::vector<std::string> _time;
    timeClient.update();

    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();

    int _0, _1, _2, _3;

    _1 = hours % 10;
    hours /= 10;
    _0 = hours % 10;

    _3 = minutes % 10;
    minutes /= 10;
    _2 = minutes % 10;

    _time = {std::to_string(_0), std::to_string(_1), std::to_string(_2), std::to_string(_3),};

    Serial.println(_0);
    Serial.println(_1);
    Serial.println(_2);
    Serial.println(_3);

    return _time;
}

std::vector<int> getIndex(std::vector<std::string> s) {
    std::vector<int> index;

    for (int i = 0; i < s.size(); ++i) {
        for (int j = 0; j < contents[i].size(); ++j) {
            int k = 0;
            while (contents[i][j][k++] != ' ');

            int n = contents[i][j].size() - k;
            auto content = contents[i][j].substr(k, n);
            if (content == s[i]) {
                index.push_back(j);
                break;
            }
        }

    }

    return index;
}