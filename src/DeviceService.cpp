//
// Created by liuke on 2023/8/19.
//

#include "DeviceService.h"

int base_step = 17;
extern std::map<int, int> addr_map;
extern std::map<int, std::pair<int, int>> zero_pos; // addr, index, offset
extern std::vector<std::vector<std::string>> contents;

extern std::map<int, int> flip_pos;
extern std::map<int, int> switch_status;
extern std::map<std::string, std::vector<std::string>(*)()> providers;
extern std::map<std::string, std::map<int, int>> phrase;
std::map<int, int> passing_mag;

Stepper stepper = Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);


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
    set_switch_from_status();

    while (!std::all_of(switch_status.begin(), switch_status.end(), [](std::pair<int, int> p) {
        return p.second == 0;
    })) {
        move_step(base_step);
        for (auto &item: switch_status) {
            if (not_on_mag(item.first)) {
                switch_status[item.first] = 0;
                set_switch(item.first, 0);
            }
        }
    }

    for (auto &item: switch_status) {
        if (not_on_mag(item.first))item.second = 1;
        else item.second = 0;
    }
    set_switch_from_status();

    while (!std::all_of(switch_status.begin(), switch_status.end(), [](std::pair<int, int> p) {
        return p.second == 0;
    })) {
        move_step(1.0);
        for (auto &item: switch_status) {
            if (!not_on_mag(item.first)) {
                switch_status[item.first] = 0;
                set_switch(item.first, 0);
            }
        }
    }

    for (const auto &item: zero_pos) {
        move_offset(item.first);
        flip_pos[item.first] = item.second.second;
    }

    release_motor();

    return 0;
}


int move_offset(int addr) {
    int _offset = zero_pos[addr].second;

    for (const auto &item: addr_map) {
        if (item.first != addr)set_switch(item.first, LOW);
        else set_switch(addr, HIGH);
    }
    passing_mag[addr] = 1;
    move_step(_offset);

    return 0;
}

int move_step(int step) {

    if (step == 0)return 0;

    if (step < 0)step = -step;

    for (const auto &item: switch_status) {
        if (item.second == 1) {
            flip_pos[item.first] += step;
        }
    }

    stepper.step(-step);

    return step;
}

int set_switch_from_status() {
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
    if (addr == MASTER_ADDR) {
        digitalWrite(CTRL, state);
        return 0;
    } else {
        Wire.beginTransmission(addr);
        Wire.write(state);
        int code = Wire.endTransmission();
        return code;
    }
}

int not_on_mag(int addr) {
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

    for (const auto &item: addr_map) {
        if (item.first != addr)set_switch(item.first, LOW);
        else set_switch(addr, HIGH);
    }

    while (!not_on_mag(addr)) {
        move_step(6);
    }
    while (not_on_mag(addr)) {
        move_step(6);
    }

    release_motor();

    flip_pos[addr] = 0;
    zero_pos[addr].second = 0;
    return 0;
}

int release_motor() {
    for (const auto &item: addr_map) {
        set_switch(item.first, LOW);
        switch_status[item.first] = 0;
    }

//    digitalWrite(IN1, LOW);
//    digitalWrite(IN2, LOW);
//    digitalWrite(IN3, LOW);
//    digitalWrite(IN4, LOW);

    return 0;
}


int get_current_index(int addr) {
    int i = 0;
    if (zero_pos[addr].second <= flip_pos[addr] and flip_pos[addr] <= stepsPerRevolution) {
        i = (flip_pos[addr] - zero_pos[addr].second) / stepPerFlip;
    } else if (0 <= flip_pos[addr] and flip_pos[addr] <= zero_pos[addr].second) {
        i = 39;
    }

    i = (i + zero_pos[addr].first) % 40;

    Serial.print("step: ");
    Serial.print(flip_pos[addr]);
    Serial.print(", addr: ");
    Serial.print(addr);
    Serial.print(", now index: ");
    Serial.print(i);
    Serial.print(", now content: ");
    Serial.println(contents[addr_map[addr]][i].c_str());

    return i;
}

int move_to_flip(const std::map<int, int> &target) {

    std::map<int, int> current_index;
    std::map<int, int> target_step;

//    for (auto &item: switch_status) {
//        item.second = 0;
//    }
//    set_switch_from_status();


    for (const auto &item: target) {
        current_index[item.first] = get_current_index(item.first);
        int move_flip_num = item.second - current_index[item.first];
        if (move_flip_num < 0)move_flip_num = 40 + move_flip_num;
        target_step[item.first] = (move_flip_num * stepPerFlip + flip_pos[item.first]) % stepsPerRevolution;

//        set_switch(item.first, 1);
//        move_step(move_flip_num * stepPerFlip);
//        set_switch(item.first, 0);


        Serial.print("addr: ");
        Serial.print(item.first);
        Serial.print(", move_flip_num ");
        Serial.print(move_flip_num);
        Serial.print(", target: ");
        Serial.println(target_step[item.first]);

    }

//    return 0;

    auto on_target = [&](const std::pair<int, double> &item) {
        return abs(target_step[item.first] - item.second) < base_step;
    };

    for (auto &item: flip_pos) {
        if (on_target(item)) {
            switch_status[item.first] = 0;
            Serial.print(item.first);
            Serial.println(" on target, set 0");
        } else {
            Serial.print(item.first);
            Serial.println(" not on target, set 1");
            switch_status[item.first] = 1;
        }
    }
    set_switch_from_status();

    while (!std::all_of(switch_status.begin(), switch_status.end(), [](std::pair<int, int> p) {
        return p.second == 0;
    })) {
        set_switch_from_status();
        move_step(base_step);

        for (const auto &item: flip_pos) {
            Serial.print(item.first);
            Serial.print(" : ");
            Serial.print(item.second);
            Serial.print(", target: ");
            Serial.print(target_step[item.first]);
            Serial.print(", sw: ");
            Serial.println(switch_status[item.first]);

            if (switch_status[item.first] == 1 and on_target(item)) {
                set_switch(item.first, 0);
                switch_status[item.first] = 0;
                Serial.print(item.first);
                Serial.println(" reach pos.");
            }

            if (not_on_mag(item.first)) {
                passing_mag[item.first] = 0;
            } else {
                if (passing_mag[item.first] == 0) {
                    Serial.print(item.first);
                    Serial.println(" reach mag.");
                    flip_pos[item.first] = 0;
                    move_offset(item.first);
                }
            }

        }
        Serial.println();
    }

    release_motor();

    return 0;
}