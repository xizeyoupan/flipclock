
#include "config.h"
#include "init.h"
#include "map"
#include "sstream"
#include "Controller.h"
#include "DeviceService.h"
#include "FSService.h"
#include "ContentProvider.h"

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

extern Stepper stepper;

extern std::map<int, int> addr_map;

void setup() {
    Wire.begin();
    Serial.begin(115200);

    if (SPIFFS.begin()) {
        Serial.println("SPIFFS Started.");

    } else {
        Serial.println("SPIFFS Failed to Start, Restarting...");
        ESP.restart();
    }
    init_all();

    stepper.setSpeed(digitalRead(SW2) ? HIGH_RPM : LOW_RPM);
    auto config_file = SPIFFS.open("/wificonfig");

    if (!config_file) {
        Serial.println("There was an error opening the wifi config file for reading, Restarting");
        ESP.restart();
    }

    std::string _s;
    while (config_file.available()) {
        _s += config_file.read();
    }
    std::istringstream iss(_s);
    std::string ssid, password;
    iss >> ssid >> password;

    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to ");
    Serial.println(ssid.c_str());

    while (WiFiClass::status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\nIP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("WiFi status:");
    WiFi.printDiag(Serial);


    if (!MDNS.begin("flip")) {
        Serial.println("Error setting up MDNS responder!");
    }

    scan_device_and_content_provider();

    std::string config;
    read_config(CONFIG_PATH, config);
    Serial.println("Config:");
    Serial.println(config.c_str());

    Serial.println("reset pos...");
    move_to_zero_pos_all();

    Serial.println("init peripheral...");
    init_external_peripheral();

    Serial.println("init server...");
    init_server();

}

void loop() {

    handle_url();
    display_seq();
//    Serial.println(ESP.getFreeHeap());
    delay(1);

}