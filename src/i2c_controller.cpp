//
// Created by liuke on 2023/8/19.
//

#include "i2c_controller.h"

std::map<int, int> devices_scan() {
    std::map<int, int> _addr;
    for (uint8_t i = 1; i < 128; i++) {
        Wire.beginTransmission(i);
        int code = Wire.endTransmission();
        delay(1);
        if (code == 0) {
            _addr.insert({i, 0});
        }
        Serial.print("Addr: ");
        Serial.print(i);
        Serial.print(", code: ");
        Serial.println(code);
    }
    return _addr;
}
