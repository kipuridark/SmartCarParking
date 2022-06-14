#include "ThingsBoard.h"
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);

#define WIFI_AP             "UiTiOt-E3.1"
#define WIFI_PASSWORD       "UiTiOtAP"

#define TOKEN               "229MVgmnXVGEkVB5qS8e"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

#define SERIAL_DEBUG_BAUD   115200

WiFiClient espClient;
ThingsBoard tb(espClient);
int status = WL_IDLE_STATUS;
Servo servoin;           //servo as entrance gate
Servo servoout;          //servo as exit gate

int carEntrance = D0;                 // Entrance sensor
int carExit = D7;                     // Exit sensor
int slot2 = D6;				  // Slot 2 sensor
int slot1  = D2;                      // Slot 1 sensor
int count = 0, sumin = 0, sumout = 0;

int entrysensor, exitsensor, s1, s2, s3;
boolean s1_occupied = false;
boolean s2_occupied = false;
boolean s3_occupied = false;
void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);

  InitWiFi();

  servoout.attach(D1);      // servo exit pin to D1
  servoin.attach(D5);       // servo entrance pin to D5

  pinMode(carExit, INPUT);
  pinMode(carEntrance, INPUT);
  pinMode(slot1, INPUT);
  pinMode(slot2, INPUT);

  Wire.begin(D4,D3);
  lcd.begin();
  lcd.backlight();   // turn on LCD backlight
}

void loop() {

  //Connect to wifi
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
  }
  //Connect to thingsboard
  if (!tb.connected()) {
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }
  delay(1000);
  Serial.println("Sending data...");

  entrysensor = digitalRead(carEntrance);
  exitsensor = digitalRead(carExit);
  s1 = digitalRead(slot1);
  s2 = digitalRead(slot2);
  lcd.setCursor(0, 0);
  lcd.print("Cho trong  ");
  lcd.setCursor(11, 0);
  int trong = 2 - count;
  lcd.print(trong);
  if (entrysensor == 0) {
    if (count >= 0 && count < 2 )
    {
      count =  count + 1;
      sumin = sumin + 1;
      servoin.write(180);
      lcd.setCursor(0, 1);
      lcd.print("Kinh chao quy khanh");
      delay(2000);
      servoin.write(90);
      lcd.clear();
      if (count >= 2) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Bai xe khong con cho trong");
        Serial.println("Bai xe da full");
        count = 2;
        delay(1500);
        lcd.clear();
      }

    }
    Serial.print("So xe trong bai: ");
    Serial.print(count);
    Serial.print("\n");


  }

  if (exitsensor == 0) {
    if (count > 0 && count < 3 ) {
      count = count - 1;
      if (count < 0) {
        count = 0;
        Serial.print("So xe trong bai: ");
        Serial.print(count);
      }
      servoout.write(180);
      sumout = sumout + 1;
      lcd.setCursor(0, 1);
      lcd.print("Tam biet quy khach");
      delay(2000);
      servoout.write(90);
      Serial.println("So xe trong bai: ");
      Serial.print(count);
      lcd.clear();
    }
  }

  if (s1 == 1 && s1_occupied == false) {
    Serial.println("Available1 ");
    s1_occupied = true;
  }

  if (s1 == 0 && s1_occupied == true) {
    Serial.println("Occupied1 ");
    s1_occupied = false;
  }

  if (s2 == 1 && s2_occupied == false) {
    Serial.println("Available2 ");
    s2_occupied = true;
  }

  if (s2 == 0 && s2_occupied == true) {
    Serial.println("Occupied2 ");
    s2_occupied = false;
  }

  tb.sendTelemetryInt("Xe trong bai", count);
  tb.sendTelemetryInt("Tong luong xe vao", sumin);
  tb.sendTelemetryInt("Tong luong xe ra", sumout);
  tb.sendTelemetryBool("Slot 1 con trong", s1_occupied);
  tb.sendTelemetryBool("Slot 2 con trong", s2_occupied);
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}
