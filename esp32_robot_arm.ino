#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

WebServer Server;
AutoConnect Portal(Server);
HTTPClient http;

unsigned pin[] = {18, 12, 26, 32, 22, 4};
unsigned clk[] = {19, 13, 27, 33, 23, 5};

String serverName = "http://161.246.6.20:110";

unsigned long lastTime = 0;
unsigned long timerDelay = 50;

int servo[] = {-1, -1, -1, -1, -1, -1};

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
  http.begin(serverName.c_str());
  
  for (int i = 0; i < 6; i++) {
    pinMode(pin[i], OUTPUT);
    pinMode(clk[i], OUTPUT);
  }
}

void loop() {
  Portal.handleClient();
  
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        String payload = http.getString();

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();

        for (int i = 0; i < 6; i++) {
          uint16_t n = obj["payload"][i];

          if (servo[i] != n) {
            servo[i] = n;
            for (int j = 0; j < 16; j++) {
              uint16_t temp = n & 0x8000;
              temp >>= 15;
              Serial.print(temp);
              digitalWrite(pin[i], temp);
              digitalWrite(clk[i], HIGH);
              delayMicroseconds(10);
              digitalWrite(clk[i], LOW);
              delayMicroseconds(10);
              n <<= 1;
            }
            Serial.println();
          }
        }
      }
      http.end();
    }
    lastTime = millis();
  }
}
