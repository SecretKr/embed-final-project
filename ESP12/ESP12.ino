// Wifi
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

// Firebase
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define DATABASE_URL "https://embed-final-project-d2b6c-default-rtdb.asia-southeast1.firebasedatabase.app"
FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;
#include "arduino_secrets.h"

// NTP Time
#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
const long  gmtOffset_sec = 25200;
NTPClient timeClient(ntpUDP, "pool.ntp.org", gmtOffset_sec);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("ESP12-wifi-setup","password");
  if(!res) Serial.println("Failed to connect");
  else Serial.println("Connected");
  
  config.api_key = FIREBASE_API;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase OK");
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Serial.available() > 0) {
    String str = Serial.readString();
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    FirebaseJson json;
    String lat = "";
    String lon = "";
    String pm = "";
    int st = 0;
    for(int i = 0;i < str.length();i++){
      if(str[i] == ',') st++;
      else{
        if(st == 0) lat += str[i];
        if(st == 1) lon += str[i];
        if(st == 2) pm += str[i];
      }
    }
    json.set("lat", lat);
    json.set("lon", lon);
    json.set("pm", pm);
    Serial.println("sending " + lat + ", " + lon + ", " + pm);
    if(!Firebase.RTDB.set(&fbdo, "logs/"+ String(epochTime), &json)) Serial.println("Firebase set log error");
  }
}
