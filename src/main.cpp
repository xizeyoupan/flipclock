#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Stepper.h>
#include <WiFiManager.h>

#define LED_PIN         4
#define NUMPIXELS       1

#define IN1             23
#define IN2             25
#define IN3             26
#define IN4             27
#define CTRL            17

#define SW1             33
#define SW2             32

#define HIGH_RPM        15
#define LOW_RPM         5

const int stepsPerRevolution = 2038;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool flag1 = true, flag2 = flag1;

WiFiManager wifiManager;

void IRAM_ATTR random_led() {
    flag1 = !flag1;
}

void IRAM_ATTR get_sw_value() {
    int high_speed = digitalRead(SW2);
    int motor_enable = digitalRead(SW1);

    digitalWrite(CTRL, motor_enable);
    myStepper.setSpeed(high_speed ? HIGH_RPM : LOW_RPM);
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(CTRL, OUTPUT);

    pinMode(0, INPUT);
    pinMode(SW1, INPUT);
    pinMode(SW2, INPUT);
    attachInterrupt(digitalPinToInterrupt(0), random_led, FALLING);
    attachInterrupt(digitalPinToInterrupt(SW1), get_sw_value, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SW2), get_sw_value, CHANGE);

    myStepper.setSpeed(digitalRead(SW2) ? HIGH_RPM : LOW_RPM);

    digitalWrite(CTRL, digitalRead(SW1));
    Serial.begin(115200);
    pixels.begin();

    wifiManager.autoConnect();
}

void loop() {

    myStepper.step(stepsPerRevolution / 360);

    if (flag1 != flag2) {
        flag2 = flag1;
        pixels.clear();
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(random(0, 15), random(0, 15), random(0, 15)));
        pixels.show();
    }
//    delay(100);
}

