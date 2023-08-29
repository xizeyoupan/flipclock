//
// Created by liuke on 2023/8/19.
//

#include "DeviceService.h"

extern std::map<int, int> addr_map;
extern std::map<int, std::pair<int, double>> zero_pos; // addr, index, offset
extern std::vector<std::vector<std::string>> contents;

extern std::map<int, double> flip_degree;
std::map<int, int> switch_status;
extern std::map<std::string, std::vector<std::string>(*)()> providers;
extern std::map<std::string, std::map<int, int>> phrase;

Stepper stepper = Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

double step_remainder = 0;

std::map<int, int> scan_device_and_content_provider() {

    int _index = 0;
    for (uint8_t i = 1; i < 128; i++) {
        Wire.beginTransmission(i);
        int code = Wire.endTransmission();
        delay(2);
        if (code == 0) {
            addr_map.insert({i, _index++});
            switch_status[i] = 0;
        }
    }
    addr_map.insert({MASTER_ADDR, _index++});
    switch_status[MASTER_ADDR] = 0;

    Serial.print("Found ");
    Serial.print(addr_map.size());
    Serial.println(" devices.");

    for (const auto &item: addr_map) {
        Serial.print("Addr: ");
        Serial.print(item.first);
        Serial.print(", index: ");
        Serial.println(item.second);
    }

    for (const auto &item: providers) {
        phrase[item.first] = {};
    }

    return addr_map;
}

int move_to_zero_pos_all() {

    for (auto &item: switch_status) {
        if (not_on_mag(item.first))item.second = 0;
        else item.second = 1;
    }
    set_switch();

    while (!std::all_of(switch_status.begin(), switch_status.end(), [](std::pair<int, int> p) {
        return p.second == 0;
    })) {
        move_degree(1.0);
        for (auto &item: switch_status) {
            if (not_on_mag(item.first)) {
                item.second = 0;
                set_switch(item.first, 0);
            }
        }
    }

    for (auto &item: switch_status) {
        if (not_on_mag(item.first))item.second = 1;
        else item.second = 0;
    }
    set_switch();

    while (!std::all_of(switch_status.begin(), switch_status.end(), [](std::pair<int, int> p) {
        return p.second == 0;
    })) {
        move_degree(1.0);
        for (auto &item: switch_status) {
            if (!not_on_mag(item.first)) {
                item.second = 0;
                set_switch(item.first, 0);
            }
        }
    }

    for (const auto &item: zero_pos) {
        move_offset(item.first);
        flip_degree[item.first] = item.second.second;
    }

    release_motor();

    return 0;
}


int move_offset(int addr) {
    double _offset = zero_pos[addr].second;

    for (const auto &item: addr_map) {
        if (item.first != addr)set_switch(item.first, LOW);
        else set_switch(addr, HIGH);
    }

    move_degree(_offset);

    return 0;
}

double move_degree(double degree) {

    if (abs(degree) < 1e-2)return 0;

    if (degree < 0)degree = -degree;
    double steps = 1.0 * degree * stepsPerRevolution / 360.0;

    int num = floor(steps);

    double _real_degree = 1.0 * num * 360.0 / stepsPerRevolution;

    for (const auto &item: switch_status) {
        if (item.second == 1) {
            flip_degree[item.first] += _real_degree;
            flip_degree[item.first] = fmod(flip_degree[item.first], 360.0);
        }
    }

    step_remainder += steps - num;
    if (step_remainder > 1) {
        step_remainder--;
        num++;
    }
    stepper.step(-num);

    return _real_degree;
}

int set_switch() {
    for (const auto &item: switch_status) {
        if (item.first == MASTER_ADDR) {
            digitalWrite(CTRL, item.second);
        } else {
            Wire.beginTransmission(item.first);
            Wire.write(item.second);
            int code = Wire.endTransmission();
        }
    }
    return 0;
}

int set_switch(int addr, int state) {
    if (addr_map.find(addr) == addr_map.end())
        return -1;

    switch_status[addr] = state;

    if (addr == MASTER_ADDR) {
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

    int pos = -1;

    if (addr == MASTER_ADDR) {
        pos = digitalRead(MAG_PIN);
    } else {
        Wire.requestFrom(addr, 1);
        while (Wire.available()) {
            pos = Wire.read();
        }
    }

    return pos;
}


int reset_pos(int addr) {
    if (addr_map.find(addr) == addr_map.end())
        return -1;

    for (const auto &item: addr_map) {
        if (item.first != addr)set_switch(item.first, LOW);
        else set_switch(addr, HIGH);
    }


    while (!not_on_mag(addr)) {
        move_degree(1);
    }
    while (not_on_mag(addr)) {
        move_degree(1);
    }

    release_motor();

    flip_degree[addr] = 0;
    return 0;
}

int release_motor() {
    for (const auto &item: addr_map) {
        set_switch(item.first, LOW);
    }

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    return 0;
}


int get_current_index(int addr) {
    int i = 0;
    if (zero_pos[addr].second <= flip_degree[addr] and flip_degree[addr] <= 360.0) {
        i = (flip_degree[addr] - zero_pos[addr].second) / degreePerFlip;
    } else if (0 <= flip_degree[addr] and flip_degree[addr] <= zero_pos[addr].second) {
        i = 39;
    }

    i = (i + zero_pos[addr].first) % 40;
    return i;
}

int move_to_flip(std::vector<int> index) {
    return 0;
}