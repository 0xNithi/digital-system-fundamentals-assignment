#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid     = "Redz-WiFi";
const char* password = "redze212224";

HTTPClient http;

const byte pin[] = {18, 12, 26, 32, 22};
const byte clk[] = {19, 13, 27, 33, 23};

const String serverName = "http://161.246.6.20:110";

unsigned long lastTime = 0;
const unsigned long timerDelay = 50;

uint8_t servo[] = { 65, 145, 140, 140, 155};

void moveServo(uint16_t (&arr)[6]);

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 5; i++) {
    pinMode(pin[i], OUTPUT);
    pinMode(clk[i], OUTPUT);
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  http.begin(serverName.c_str());

  for (int i = 0; i < 5; i++) {
    uint16_t temp1 = (uint16_t)servo[i];
    for (int j = 0; j < 16; j++) {
      uint16_t temp2 = temp1 & 0x8000;
      temp2 >>= 15;
      digitalWrite(pin[i], temp2);
      digitalWrite(clk[i], HIGH);
      delayMicroseconds(10);
      digitalWrite(clk[i], LOW);
      delayMicroseconds(10);
      temp1 <<= 1;
    }
  }
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        String payload = http.getString();

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();

        moveServo(obj);
      }
      http.end();
    }
    lastTime = millis();
  }
}

void moveServo(JsonObject &obj) {
  for (int i = 0; i < 5; i++) {
    uint16_t n = obj["payload"][i];

    while (servo[i] != n) {
      if (servo[i] < n) {
        servo[i] += 1;
      } else {
        servo[i] -= 1;
      }
      uint16_t temp1 = (uint16_t)servo[i];
      for (int j = 0; j < 16; j++) {
        uint16_t temp2 = temp1 & 0x8000;
        temp2 >>= 15;
        Serial.print(temp2);
        digitalWrite(pin[i], temp2);
        digitalWrite(clk[i], HIGH);
        delayMicroseconds(25);
        digitalWrite(clk[i], LOW);
        delayMicroseconds(25);
        temp1 <<= 1;
      }
      Serial.println();
      delay(obj["payload"][5]);
    }
  }
}
