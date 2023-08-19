
#include "config.h"
#include "ArduinoJson.h"
#include "init.h"
#include "map"
#include "url_controller.h"

DynamicJsonDocument doc(2048);
AsyncWebServerRequest *global_request = nullptr;

const char *ssid = "";
const char *password = "";

Stepper stepper = Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
AsyncWebServer server(80);

std::string url_handle;
std::string post_data;
bool post_data_end;

std::map<int, int> addr;

void setup() {
    SPIFFS.begin();
    Wire.begin();
    Serial.begin(115200);

    init_all();

    stepper.setSpeed(digitalRead(SW2) ? HIGH_RPM : LOW_RPM);

    WiFi.begin(ssid, password);
    Serial.println(String("Connecting to ") + ssid);

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

    init_server(server, url_handle, &global_request, post_data, post_data_end);

}

void loop() {

    handle_url(global_request, url_handle, doc, stepper, post_data, post_data_end);
    delay(1);

}