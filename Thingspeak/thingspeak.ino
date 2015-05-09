#include <Wire.h>

// ----- WEB
// http://www.homautomation.org/2013/10/01/playing-with-arduino-yun/

#include <Bridge.h>;
#include <YunServer.h>;
#include <YunClient.h>;
// All request on http://***ard.ip***/arduino/
// will be transfered here
YunServer server;
String startString;

// ----- ThingsPeak - https://thingspeak.com/channels/35264
#include "HttpClient.h"

//Thingspeak parameters http://blogspot.tenettech.com/?p=3110    
String thingspeak_update_API    = "http://api.thingspeak.com/update";  // https://gist.github.com/netmaniac/10930106
String thingspeak_write_API_key = "?key=****SUPER SECRET KEY***";//Insert Your Write API key here
String thingspeakfieldstart      = "&field";
String thingspeakfieldend     = "=";
unsigned int currentThingsPeak = 0;
unsigned long startThingsPeak;

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
#define DHTPIN 8         // what pin we're connected to 
#define DHTTYPE DHT22    // DHT 22  (AM2302) 
DHT dht(DHTPIN, DHTTYPE);

float h;      // Humidity
float t;      // Celsius    
float f;      // Fahrenheit
// Heatindex, fahrenheit, skrivs om till celsius raden under.
float hi;
float hc;

// ----- PushButton för backlight
// Set pins.
#define BUTTON_PIN A5            // The number of the push-button pin.
#define LCD_LIGHT_PIN A4         // The number of the pin where anode of the display backlight is.
#define LCD_LIGHT_ON_TIME 10000  // How long (in milliseconds) should lcd light stay on?

unsigned int currentLcdLightOnTime = 0;
unsigned long lcdLightOn_StartMillis;    // For calculating the lcd light on time.
boolean isLcdLightOn;

// For checking push-button state.
int buttonState = 0;

// ----- LCD-uppdatering
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);     // pins, 12 och 11 uppfyller vissa krav...

byte grad[8] = {        // gradersymbol
  B00110,
  B01001,
  B00110,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte pil[8] = {      // pil nedåt från vänster
  B00000,
  B11100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
};

void setup() {
  // ----- LCD-uppdatering
  lcd.createChar(0, grad);      // skapa symbolerna
  lcd.createChar(1, pil);
  Serial.begin(9600);
  lcd.begin(20, 4);             // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();                  // start with a blank screen

  // ----- WEB // ----- ThingsPeak
  Bridge.begin();
  delay(1000);

  // Listen for incoming connection only from localhost - no one from the external network could connect
  server.listenOnLocalhost();
  server.begin();

  // get the time that this sketch started:
  Process startTime;
  startTime.runShellCommand("date");
  while (startTime.available()) {
    char c = startTime.read();
    startString += c;
  }

  // ----- DHT-avläsning
  dht.begin();

  // ----- PushButton för backlight
  pinMode(BUTTON_PIN, INPUT);
  // Set the lcd display backlight anode pin as an output.
  pinMode(LCD_LIGHT_PIN, OUTPUT);
  // Set the lcd display backlight anode pin to low - lcd light off.
  digitalWrite(LCD_LIGHT_PIN, LOW);
  isLcdLightOn = false;

  delay(1000);
}

void loop() {
  // ----- Tiden (Uptime - LCD)
  int d = day() - 1;      // korrigerad
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
  h = dht.readHumidity();        // måste nog korrigeras med +4
  // Celsius
  t = dht.readTemperature();
  // Fahrenheit
  f = dht.readTemperature(true);
  // Heatindex, fahrenheit, skrivs om till celsius raden under.
  hi = dht.computeHeatIndex(f, h);
  hc = (hi - 32) / 1.8;

  delay(250);   // tid för att läsa av, typ. 

  if (isLcdLightOn) {  // Uppdaterar LCD'n bara om skärmen är på.
    // ----- LCD-uppdatering
    lcd.clear();
    lcd.setCursor(0, 0);  // Rad 1
    lcd.print("Luftfuktighet:");
    lcd.print(h);
    lcd.print("%");

    lcd.setCursor(0, 1);  // Rad 2
    lcd.print("Temperatur: ");
    lcd.print(t); lcd.write(byte(0)); lcd.print("C");   // gradersymbolen.

    lcd.setCursor(0, 2);  // Rad 3
    lcd.print("Vaken"); lcd.write(byte(1)); lcd.print("|");
    lcd.setCursor(7, 2);
    lcd.print("V"); lcd.print((char)225); lcd.print("rme:");  // bokstaven ä, använder client.print("\x00E4"); för web
    lcd.print(hc); lcd.write(byte(0)); lcd.print("C");

    lcd.setCursor(0, 3);  // Rad 4
    lcd.print(d); lcd.print(" d ");
    lcd.print(H); lcd.print(" h ");
    lcd.print(m); lcd.print(" min ");
    lcd.print(s); lcd.print(" s");
  }
  
  // ----- WEB
  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
  if (client) {
    // read the command
    String command = client.readString();
    command.trim();        //kill whitespace
    Serial.println(command);
    // is "temperature" command?
    if (command == "Humidor") {
      // get the time from the server:
      Process time;
      time.runShellCommand("date");
      String timeString = "";
      while (time.available()) {
        char c = time.read();
        if (c != '\n') {
          timeString += c;
        }
      }
      client.print("Datum och tid: "); client.print(timeString); client.println("");
      client.print("Temperatur:    "); client.print(t); client.print("\x00B0"); client.println("C");
      client.print("Luftfuktighet: "); client.print(h); client.println(" %");
      client.print("V"); client.print("\x00E4"); client.print("rme:         "); client.print(hc); client.print("\x00B0"); client.println("C");
      client.print("Uptime:        "); client.print(d); client.print(" d, "); client.print(H); client.print(" h, "); client.print(m); client.print(" min, "); client.print(s); client.print(" s.");
    }
    // Close connection and free resources.
    client.stop();
  }

  // ----- ThingsPeak
  int tT = dht.readTemperature() * 10;
  int hT = dht.readHumidity() * 10;
  if (currentThingsPeak > 60000) {
    postToThinspeak(tT, 1, hT, 2);
    currentThingsPeak = 0;
    startThingsPeak = millis();
  }
  else {
    currentThingsPeak = millis() - startThingsPeak;
  }
  delay(50); // Poll every 50ms
}

void postToThinspeak(int T, int F1, int H, int F2) {
  HttpClient client;
  String request_string =  thingspeak_update_API + thingspeak_write_API_key + thingspeakfieldstart + F1 + thingspeakfieldend  + T + thingspeakfieldstart + F2 + thingspeakfieldend  + H;
  // Make a HTTP request:
  client.get(request_string);
  while (client.available()) {  // för att kolla värden
    char c = client.read();
    Serial.println(c);
    Serial.println(request_string);  // för att kontrollera skickat.
    delay(500);
  }
}
