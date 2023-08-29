//
// Created by liuke on 2023/8/19.
//

#ifndef FLIPCLOCK_CONFIG_H
#define FLIPCLOCK_CONFIG_H

#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include "Wire.h"
#include "SPIFFS.h"
#include "Stepper.h"
#include "Adafruit_NeoPixel.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <map>

#define LED_PIN         4
#define NUMPIXELS       1

#define IN1             23
#define IN2             25
#define IN3             26
#define IN4             27
#define CTRL            17
#define MAG_PIN         16

#define SW1             33
#define SW2             32

#define HIGH_RPM        15
#define LOW_RPM         10

#define MASTER_ADDR     114

#define CONFIG_PATH     "/config"


const int stepsPerRevolution = 2038;
const double degreePerFlip = 360.0 / 40.0;

#endif //FLIPCLOCK_CONFIG_H
