//
// Created by liuke on 2023/8/19.
//

#include "DeviceService.h"

std::map<int, int> scan_devices() {
    std::map<int, int> _addr;
    int _index = 0;
    _addr.insert({0, _index++});
    for (uint8_t i = 1; i < 128; i++) {
        Wire.beginTransmission(i);
        int code = Wire.endTransmission();
        delay(2);
        if (code == 0) {
            _addr.insert({i, _index++});
        }
    }
    Serial.print("Found ");
    Serial.print(_addr.size());
    Serial.println(" devices.");

    for (const auto &item: _addr) {
        Serial.print("Addr: ");
        Serial.print(item.first);
        Serial.print(", index: ");
        Serial.println(item.second);
    }
    return _addr;
}

void move_test(Stepper &stepper, int move) {
    stepper.step(stepsPerRevolution / move);
}
