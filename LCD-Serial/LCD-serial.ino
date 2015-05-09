/*
Humidor med knapp för backlight och dimmer förtextljus.
*/

#include <Wire.h>

// ----- LCD-uppdatering
  #include <FastIO.h>
  #include <I2CIO.h>
  #include <LCD.h>
  #include <LiquidCrystal.h>
  #include <LiquidCrystal_I2C.h>
  #include <LiquidCrystal_SR.h>
  #include <LiquidCrystal_SR2W.h>
  #include <LiquidCrystal_SR3W.h>
// ----- Tiden
  #include <Time.h>

// ----- DHT-avläsning
  #include "DHT.h"
  #define DHTPIN 8     // what pin we're connected to 
  #define DHTTYPE DHT22   // DHT 22  (AM2302) 
  DHT dht(DHTPIN, DHTTYPE);

// ----- PushButton för backlight
// Set pins.
  #define BUTTON_PIN A5    // The number of the push-button pin.
  #define LCD_LIGHT_PIN A4 // The number of the pin where anode of the display backlight is.
  
  #define LCD_LIGHT_ON_TIME 10000 // How long (in milliseconds) should lcd light stay on?
  
  unsigned int currentLcdLightOnTime = 0;
  // For calculating the lcd light on time.
  unsigned long lcdLightOn_StartMillis;
  
  boolean isLcdLightOn;
  
  // For checking push-button state.
  int buttonState = 0;

// ----- LCD-uppdatering
  LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
  
    float h;
  // Read temperature as Celsius
  float t;
  // Read temperature as Fahrenheit
  float f;
  // Heatindex, fahrenheit, skrivs om till celsius raden under.
  float hi;
  float hc;
  
  byte grad[8] = {
    B00110,
    B01001,
    B00110,
    B00000,
    B00000,
    B00000,
    B00000,
  };

void setup() {
  // ----- LCD-uppdatering
    lcd.createChar(0, grad);
    Serial.begin(9600);
    lcd.begin(20, 4);             // columns, rows.  use 16,2 for a 16x2 LCD, etc.
    lcd.clear();                  // start with a blank screen

  // ----- DHT-avläsning
    dht.begin();

  // ----- PushButton för backlight
    pinMode(BUTTON_PIN, INPUT);
    // Set the lcd display backlight anode pin as an output.
    pinMode(LCD_LIGHT_PIN, OUTPUT);
    // Set the lcd display backlight anode pin to low - lcd light off.
    digitalWrite(LCD_LIGHT_PIN, LOW);
    isLcdLightOn = false;
}

void loop() {
  // ----- Tiden
  int d = day() - 1;
  int H = hour();
  int m = minute();
  int s = second();

  // ----- PushButton för backlight
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == HIGH) {  // Button pressed.
    lcdLightOn_StartMillis = millis();
    currentLcdLightOnTime = 0;
    isLcdLightOn = true;
    digitalWrite(LCD_LIGHT_PIN, HIGH);
  }
  else {
    if (isLcdLightOn) {
      currentLcdLightOnTime = millis() - lcdLightOn_StartMillis;
      if (currentLcdLightOnTime > LCD_LIGHT_ON_TIME) {
        isLcdLightOn = false;
        digitalWrite(LCD_LIGHT_PIN, LOW);
        lcd.clear();
      }
    }
  }
  

  // ----- DHT-avläsning
   h = dht.readHumidity();
  // Read temperature as Celsius
   t = dht.readTemperature();
  // Read temperature as Fahrenheit
   f = dht.readTemperature(true);
  // Heatindex, fahrenheit, skrivs om till celsius raden under.
   hi = dht.computeHeatIndex(f, h);
   hc = (hi - 32) / 1.8;
  
  delay(500);
  
  if (isLcdLightOn) {  // Uppdaterar LCD'n bara om skärmen är på.
    // ----- LCD-uppdatering
    lcd.clear();
    lcd.setCursor(0, 0);  // Rad 1
    lcd.print("Luftfuktighet:");
    lcd.print(h);
    lcd.print("%");

    lcd.setCursor(0, 1);  // Rad 2
    lcd.print("Temperatur: ");
    lcd.print(t);
    lcd.write(byte(0)); lcd.print("C");

    lcd.setCursor(0, 2);  // Rad 3
    lcd.print("Uptime");
    lcd.setCursor(9, 2);
    lcd.print("Varme:");
    lcd.print(hc);

    lcd.setCursor(0, 3);  // Rad 4
    lcd.print(d); lcd.print(" d ");
    lcd.print(H); lcd.print(" h ");
    lcd.print(m); lcd.print(" min ");
    lcd.print(s); lcd.print(" s");
  }
}
