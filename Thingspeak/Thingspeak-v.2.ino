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
String thingspeak_write_API_key = "?key=*** SUPER SECRET KEY***";//Insert Your Write API key here
String thingspeakfieldstart      = "&field";
String thingspeakfieldend     = "=";
unsigned int currentThingsPeak = 0;     // för uppdateringsklockan
unsigned long startThingsPeak;
// För att beräkna medelvärde
long tShort = 0;
long hShort = 0;
long hcShort = 0;
int TPcount = 0;
int TPcountLong = 0;
long tLong = 0;
long hLong = 0;
long hcLong = 0;

// ----- LCD-uppdatering
#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>

// ----- Tiden
#include <Time.h>

// ----- DHT-avläsning
#include "DHT.h"
#define DHTPIN 8         // what pin we're connected to 
#define DHTTYPE DHT22    // DHT 22  (AM2302) 
DHT dht(DHTPIN, DHTTYPE);
/*
float h;      // Humidity
float t;      // Celsius
float f;      // Fahrenheit
// Heatindex, fahrenheit, skrivs om till celsius raden under.
float hi;
float hc;*/

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
  float h = dht.readHumidity();        // måste nog korrigeras med +4
  // Celsius
  float t = dht.readTemperature();
  // Fahrenheit
  float f = dht.readTemperature(true);
  // Heatindex, fahrenheit, skrivs om till celsius
  float hc = (dht.computeHeatIndex(f, h) - 32) / 1.8;

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
    lcd.print(t); lcd.print((char)0); lcd.print("C");   // gradersymbolen. write(byte(0) ger ingen symbol, men olika gör!?!?

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
  if (currentThingsPeak > 60000) {    // detta borde kunna lösas med array el. motsv. <---><---><---->
    tShort = tShort + t * 100;        // t, h, hc från tidigare läsning, se DHT
    hShort = hShort + h * 100;
    hcShort = hcShort + hc * 100;
    TPcount++;
    if (TPcount > 4) {         // summerar fem mätningar
      TPcountLong++;          // addera ett till longmätningen
      TPcount = 0;            // nollställer mätaren
      int tT = tShort / 5;    // beräknar medelvärdet
      int hT = hShort / 5;
      tLong = tLong + tT;
      hLong = hLong + hT;
      hcLong = hcLong + hcShort / 5;
      if (TPcountLong > 6) {   // summerar sju pentas
        TPcountLong = 0;
        int stT = tLong / 7;
        int shT = hLong / 7;
        int shcT = hcLong / 7;
        postToThinspeak5(tT, 1, hT, 2, shcT, 3, stT, 4, shT, 5);
        tLong = 0;              // nollställer räknarna
        hLong = 0;
        hcLong = 0;
      }
      else {
        postToThinspeak2(tT, 1, hT, 2);
      }
      tShort = 0;            // nollställer räknarna
      hShort = 0;
    }
    currentThingsPeak = 0;
    startThingsPeak = millis();
  }
  else {
    currentThingsPeak = millis() - startThingsPeak;
  }
}

void postToThinspeak2(int T, int F1, int H, int F2) {
  HttpClient client;
  String request_string =  thingspeak_update_API + thingspeak_write_API_key + thingspeakfieldstart + F1 + thingspeakfieldend  + T + thingspeakfieldstart + F2 + thingspeakfieldend  + H;
  // Make a HTTP request:
  client.get(request_string);
}

void postToThinspeak5(int T, int F1, int H, int F2, int HC, int F3, int sT, int F4, int sH, int F5) {
  HttpClient client;
  String request_string =  thingspeak_update_API + thingspeak_write_API_key + thingspeakfieldstart + F1 + thingspeakfieldend  + T + thingspeakfieldstart + F2 + thingspeakfieldend  + H + thingspeakfieldstart + F3 + thingspeakfieldend  + HC + thingspeakfieldstart + F4 + thingspeakfieldend  + sT + thingspeakfieldstart + F5 + thingspeakfieldend  + sH;
  // Make a HTTP request:
  client.get(request_string);
}
