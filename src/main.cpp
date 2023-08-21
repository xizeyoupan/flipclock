
#include "config.h"
#include "ArduinoJson.h"
#include "init.h"
#include "map"
#include "sstream"
#include "Controller.h"
#include "DeviceService.h"

DynamicJsonDocument doc(16 * 1024);

Stepper stepper = Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
AsyncWebServer server(80);

AsyncWebServerRequest *global_request = nullptr;
std::string url_handle;
std::string post_data;
bool post_data_end;
std::map<int, int> addr_map;
std::map<int, int> zero_pos;
std::vector<std::vector<std::string>> contents;

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

    addr_map = scan_devices();

    init_server(server);

}

void loop() {

    handle_url(doc, stepper);
//    Serial.println(ESP.getFreeHeap());
    delay(1);

}