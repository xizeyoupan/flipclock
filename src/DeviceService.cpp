//
// Created by liuke on 2023/8/19.
//

#include "DeviceService.h"

extern std::map<int, int> addr_map;
extern std::map<int, int> flip_pos;
extern Stepper stepper;

double step_remainder = 0;

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

void move_degree(int degree) {
    if (degree < 0)degree = -degree;
    double steps = 1.0 * degree * stepsPerRevolution / 360.0;

    int num = floor(steps);
    step_remainder += steps - num;
    if (step_remainder > 1) {
        step_remainder--;
        num++;
    }
    stepper.step(-num);
}

int set_switch(int addr, int state) {
    if (addr_map.find(addr) == addr_map.end())
        return -1;

    if (addr == 0) {
        digitalWrite(CTRL, state);
        return 0;
    }

    Wire.beginTransmission(addr);
    Wire.write(state);
    int code = Wire.endTransmission();
    return code;

}

int not_on_mag(int addr) {
    if (addr_map.find(addr) == addr_map.end())
        return -1;

    if (addr == 0) {
        return digitalRead(MAG_PIN);
    } else {
        Wire.requestFrom(addr, 1);
        int pos = -1;
        while (Wire.available()) {
            pos = Wire.read();
        }
        return pos;
    }
}


int reset_pos(int addr) {
    if (addr_map.find(addr) == addr_map.end())
        return -1;

    for (const auto &item: addr_map) {
        if (item.first != addr)set_switch(item.first, LOW);
    }
    set_switch(addr, HIGH);

    while (!not_on_mag(addr)) {
        move_degree(1);
    }
    while (not_on_mag(addr)) {
        move_degree(1);
    }

    return 0;
}
