//
// Created by liuke on 2023/8/19.
//

#include "init.h"
extern Stepper stepper;

void init_all() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(CTRL, OUTPUT);

    pinMode(0, INPUT);
    pinMode(SW1, INPUT);
    pinMode(SW2, INPUT);

    digitalWrite(CTRL, 1);

    attachInterrupt(SW2, []() {
        stepper.setSpeed(digitalRead(SW2) ? HIGH_RPM : LOW_RPM);
    }, CHANGE);
}
