
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Wire.h"
#include "SPIFFS.h"



AsyncWebServer server(80);

void handleRoot(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
}

void setup() {
    SPIFFS.begin();
    Serial.begin(115200);
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

    server.on("/", HTTP_GET, handleRoot);
    server.begin();

//    myStepper.setSpeed(digitalRead(SW2) ? HIGH_RPM : LOW_RPM);

//    digitalWrite(CTRL, digitalRead(SW1));

//    pixels.begin();


}

void loop() {

}