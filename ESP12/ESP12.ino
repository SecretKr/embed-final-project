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

// Serial
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // rx, tx

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
//  pinMode(16, INPUT_PULLUP);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
//  if(!digitalRead(16)){
//    Serial.println("Reset WiFi");
//    WiFi.disconnect();
//  }
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

unsigned long lastSend = 0;
unsigned long currentTime = 0;
unsigned long updateTime = 0;
long interval = 30000;
String lat;
String lon;
String pm;

void loop() {
//  if (mySerial.available() > 0) {
//    String tlat = "";
//    String tlon = "";
//    String tpm = "";
//    int st = 0;
//    char ch = mySerial.read();
//    while(ch != '\n' && mySerial.available()) {
//      if(ch == ',') st++;
//      else{
//        if(st == 0) tlat += ch;
//        if(st == 1) tlon += ch;
//        if(st == 2) tpm += ch;
//      }
//      ch = mySerial.read();
//    }
//    while(mySerial.available() > 0) mySerial.read();
//    if(tlat.length() >= 5 && tlon.length() >= 5 && tpm.length() >= 1){
//      lat = tlat;
//      lon = tlon; 
//      pm = tpm;
//      updateTime = millis();
//      Serial.println("lat: "+lat+" lon: "+lon+" pm2.5: "+pm);
//    }
//  }
  if (mySerial.available() > 0) {
    String str = mySerial.readString();
    //Serial.print(str);
    String tlat = "";
    String tlon = "";
    String tpm = "";
    int st = 0;
    for(int i = 0;i < str.length();i++){
      if(str[i] == ',') st++;
      else{
        if(st == 0) tlat += str[i];
        if(st == 1) tlon += str[i];
        if(st == 2) tpm += str[i];
      }
    }
    if(tlat.length() >= 5 && tlon.length() >= 5 && tpm.length() >= 1){
      lat = tlat;
      lon = tlon; 
      pm = tpm;
      updateTime = millis();
      Serial.println("lat: "+lat+" lon: "+lon+" pm2.5: "+pm);
    }
  }
  
  currentTime = millis();
  if (currentTime-lastSend >= interval && currentTime-updateTime < interval && lat != ""){
    lastSend = currentTime;
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    FirebaseJson json;
    json.set("lat", lat);
    json.set("lon", lon);
    json.set("pm", pm);
    Serial.print("sending " + lat + ", " + lon + ", " + pm);
    if(!Firebase.RTDB.set(&fbdo, "logs/"+ String(epochTime), &json)) Serial.println("Firebase set log error");
    Serial.println("  sent!!");
  }
//  if (mySerial.available() > 0) {
//    Serial.write(mySerial.read());
//  }
}
