/**
   BasicHTTPSClient.ino

    Created on: 14.10.2018

*/

#include <Arduino.h>
#include <ESP32Servo.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#include <WiFiClientSecure.h>

const int trigPin = 0;
const int echoPin = 4;

float duration, distance;

Servo myservo;

// Not sure if WiFiClientSecure checks the validity date of the certificate. 
// Setting clock just to be sure...
void setClock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}


WiFiMulti WiFiMulti;

void setup() {
  myservo.attach(14);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("RISD-MiscDevices", "T3chn0l0gy!");

  // wait for WiFi connection
  Serial.print("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  Serial.println(" connected");

  setClock();  
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
  Serial.print("Distance: ");
  Serial.println(distance);

  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    // client -> setCACert(rootCACertificate);
    client -> setInsecure();

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      Serial.print("[HTTPS] begin...\n");
      if (https.begin(*client, "https://idsb-iot.onrender.com")) {  // HTTPS
        https.addHeader("Content-Type", "application/json");
        Serial.print("[HTTPS] POST...\n");
        // start connection and send HTTP header
        int httpCode = https.POST("{\"name\":\"ben\", \"num\":" + String(distance) + "}");
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
  
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            Serial.println(payload);
            if (payload == "true") {
            Serial.println("Rotating servo");
            int pos = 0;
            for (pos = 0; pos <= 180; pos += 1) {  // goes from 0 degrees to 180 degrees
              // in steps of 1 degree
              myservo.write(pos);  // tell servo to go to position in variable 'pos'
              delay(30);           // waits 15ms for the servo to reach the position
            }
            for (pos = 180; pos >= 0; pos -= 1) {  // goes from 180 degrees to 0 degrees
              myservo.write(pos);                  // tell servo to go to position in variable 'pos'
              delay(30);                           // waits 15ms for the servo to reach the position
            }
            }
          }
        } else {
          Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
  
        https.end();
      } else {
        Serial.printf("[HTTPS] Unable to connect\n");
      }

      // End extra scoping block
    }
    
  
    delete client;
  } else {
    Serial.println("Unable to create client");
  }

  Serial.println();
  Serial.println("Waiting 5s before the next round...");
  delay(5000);
}
