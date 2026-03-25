
#include <DFRobot_SIM808.h>
#include <Chrono.h>

#define PIN_TX 11
#define PIN_RX 10
#define STRAP 2
#define POWER_PIN 3

#define PHONE_NUMBER "+639052329023"
SoftwareSerial mySerial(PIN_TX, PIN_RX);
DFRobot_SIM808 sim808(&mySerial);
Chrono myChrono;


int sendPhase;
double longitude;
double latitude;

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  pinMode(STRAP, INPUT_PULLUP);
  pinMode(POWER_PIN, OUTPUT);
}

void loop() {

  if (isStrapRemove()) {
    if (sendPhase == 0) {
      sendPhase = 1;
    }
  } else {
    if (sendPhase != 0) {
      sendPhase = 0;
      sim808.detachGPS();
      sim808.powerUpDown(POWER_PIN);
    }
  }
  if (sendPhase == 0) {
    //nothing todo
  } else if (sendPhase == 1) {
    //Power On
    sim808.powerReset(POWER_PIN);
    sendPhase = 2;
  } else if (sendPhase == 2) {
    //Init Module
    while (!sim808.init()) {
      delay(1000);
      Serial.print("Sim808 init error\r\n");
    }

    sendPhase = 3;
  } else if (sendPhase == 3) {
    //Power ON GPS
    if (sim808.attachGPS()) {
      sendPhase = 4;
      Serial.println("Open the GPS power success");
    } else {
      sendPhase = 5;
    }
  } else if (sendPhase == 4) {
    //Save Location
    // Serial.println("Save Location");
    if (sim808.getGPS()) {
      sendPhase = 5;

      latitude = sim808.GPSdata.lat;
      longitude = sim808.GPSdata.lon;
      sim808.detachGPS();
      myChrono.restart();
    }
  } else if (sendPhase == 5) {
    if (myChrono.hasPassed(10000)) {
      myChrono.restart();
      String msg = "Help! I'm at Latitude: " + String(latitude, 6) + " Longitude:" + String(longitude, 6);
      Serial.println(msg);
      bool result = sim808.sendSMS((char *)PHONE_NUMBER, msg.c_str());
      if (result) {
        Serial.println("Send Success");
      }
      sendPhase = 6;
    }


  } else if (sendPhase == 6) {
    //Power Off Module
    if (myChrono.hasPassed(5000)) {
      sendPhase = 7;
      sim808.powerUpDown(POWER_PIN);
      Serial.println("Done sending");
    }

  } else if (sendPhase == 7) {
    //Wait to strap to attached again
  }
}

bool isStrapRemove() {
  return digitalRead(STRAP);
}
