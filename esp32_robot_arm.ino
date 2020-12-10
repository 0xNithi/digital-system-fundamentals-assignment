#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

WebServer     Server;
AutoConnect   Portal(Server);

unsigned pin[] = {13};
unsigned clk = 12;

String serverName = "http://161.246.6.20:110";

unsigned long lastTime = 0;
unsigned long timerDelay = 50;

uint16_t servo[] = {0, 0, 0, 0, 0, 0};

void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Server.on("/", rootPage);
  if (Portal.begin()) {
    Serial.println("HTTP server:" + WiFi.localIP().toString());
  }
//  for (int i = 0; i < sizeof(pin); i++) {
    pinMode(pin[0], OUTPUT);
//  }
  pinMode(clk, OUTPUT);
}

void loop() {
  Portal.handleClient();
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println(payload);
        
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();
        
        uint16_t n = obj["payload"][0];

        if (servo[0] != n) {
          servo[0] = n;
          for (int i = 0; i < 16; i++) {
            uint16_t temp = n & 0x8000;
            temp  >>= 15;
            Serial.print(temp);
            digitalWrite(13, temp);
            digitalWrite(12, HIGH);
            delayMicroseconds(10);
            digitalWrite(12, LOW);
            delayMicroseconds(10);
            n <<= 1;
          }
        }
      }
      http.end();
    }
    lastTime = millis();
  }
}
