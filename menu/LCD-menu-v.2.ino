#include <Wire.h>
#include <LCD03.h>

// Create new LCD03 instance
LCD03 lcd;

// Pushbuttons
#define BUTTON_PIN_UP 2
int buttonStateUp = 0;
#define BUTTON_PIN_DOWN 3
int buttonStateDown = 0;
#define BUTTON_PIN_LEFT 5
int buttonStateLeft = 0;
#define BUTTON_PIN_RIGHT 4
int buttonStateRight = 0;

int menuX = 0;
int menuY = 0;
boolean menuChange = true;
in
int m = 2;

int settingsBrightness = 75;
int updateShort = 1;
int updateLong = 1;
int warningOne = 1;
int warningShort = 1;
int warningLong = 1;
/*
byte uA[8] = {      // pil nedåt från vänster
  B00100,
  B01110,
  B11111,
  B00100,
  B0010,
  B00100,
  B00000,
};

byte dA[8] = {      // pil nedåt från vänster
  B00000,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
};

// Define the custom char bytes
byte smiley[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b00000
};*/

void setup() {
  /*  lcd.createChar(0, uA);      // skapa symbolerna
    lcd.createChar(1, dA);*/
  //    lcd.createChar(0, smiley);
  Serial.begin(9600);
  lcd.begin(20, 4);

  // Clear the LCD
  lcd.backlight();
  lcd.clear();
  lcd.print("Welcome To Humidor");

  delay(2000);

  pinMode(BUTTON_PIN_UP, INPUT);
  pinMode(BUTTON_PIN_DOWN, INPUT);
  pinMode(BUTTON_PIN_LEFT, INPUT);
  pinMode(BUTTON_PIN_RIGHT, INPUT);

}

void loop() {
  buttonStateUp = digitalRead(BUTTON_PIN_UP);
  buttonStateDown = digitalRead(BUTTON_PIN_DOWN);
  buttonStateLeft = digitalRead(BUTTON_PIN_LEFT);
  buttonStateRight = digitalRead(BUTTON_PIN_RIGHT);

  // int Br = settingsBrightness * 120 / 100;
  lcd.setBrightness(settingsBrightness);

  delay(150);

  if (buttonStateUp == LOW && buttonStateDown == HIGH && menuChange == false) {
    menuY++;
    menuChange = true;
  }
  if (buttonStateUp == HIGH && buttonStateDown == LOW && menuChange == false)  {
    menuY--;
    menuChange = true;
  }
  if (buttonStateLeft == LOW && buttonStateRight == HIGH && menuChange == false)  {
    menuX--;
    menuChange = true;
  }
  if (buttonStateLeft == HIGH && buttonStateRight == LOW && menuChange == false) {
    posMenu[m] = posMenu[0];
    m++;
    posMenu[0] = 1;
    menuChange = true;
  }

  delay(150);
  if (menuChange == true) {
    lcd.clear();
    menuChange = false;
    //   switch (posMenu[1]) {
    //    case 1:
    switch (posMenu[2]) {
      case 1:
        menuTemp();
        break;
      case 2:
        switch (posMenu[3]) {
          case 1:
            menuSettingsBrightness();
            break;
          case 2:
            switch (posMenu[4]) {
              case 1:
                menuSettingsUpdateShort();
                break;
              case 2:
                menuSettingsUpdateLong();
                break;
              default:
                menuSettingsUpdate();
            }
            break;
          case 3:
            switch (posMenu[4]) {
              case 1:
                menuSettingsWarningOne();
                break;
              case 2:
                menuSettingsWarningShort();
                break;
              default:
                menuSettingsWarningLong();
            }
            break;
          default:
            menuSettingsWarning();
        }
        break;
      default:
        menuStart();
        if (posMenu[0] >= 3) posMenu[0] = 2;
        /*        }
              default:
                posMenu[1] = 1;
                posMenu[2] = 0;
                posMenu[3] = 0;
                posMenu[4] = 0;*/
    }
    if (posMenu[0] <= 0) posMenu[0] = 1;
    if (posMenu[0] >= 4) posMenu[0] = 3;
    lcd.setCursor(0, posMenu[0]);

    
    for (int x = 0; x <= 4; x ++){
      Serial.print(posMenu[x]);Serial.print(" - "); 
    }
Serial.print("m=");Serial.println(m);
  }

}

void menuStart() {
  lcd.home();
  lcd.print("Menu - Root");
  lcd.setCursor(0, 1);
  lcd.print("1:Temp. and Humidity");
  lcd.setCursor(0, 2);
  lcd.print("2:Settings");
  lcd.blink();
}

void menuTemp() {
  lcd.home();
  lcd.print("Temp and 1");
  lcd.setCursor(0, 1);
  lcd.print("Temperature: "); lcd.print("22");
  lcd.setCursor(0, 2);
  lcd.print("Humidity: "); lcd.print("76%");
  lcd.setCursor(0, 3);
  lcd.print("Heatindex: "); lcd.print("24");
  lcd.noBlink();
}

void menuSettings() {
  lcd.home();
  lcd.print("<- Settings");
  lcd.setCursor(0, 1);
  lcd.print("1:Brightness");
  lcd.setCursor(0, 2);
  lcd.print("2:Updates");
  lcd.setCursor(0, 3);
  lcd.print("3.Warning");
  lcd.blink();
}

void menuSettingsBrightness() {
  lcd.home();
  lcd.print("<-- Brightness");
  lcd.setCursor(0, 1);
  lcd.print("u");
  lcd.setCursor(0, 2);
  lcd.print(settingsBrightness); lcd.print(" %" );
  lcd.setCursor(0, 3);
  lcd.print("d");
  lcd.noBlink();
  settingsBrightness = settingsBrightness - menuY;
  menuY = 0;
  if (settingsBrightness <= 0) {
    settingsBrightness = 1;
  }
  if (settingsBrightness >= 100) {
    settingsBrightness = 99;
  }
}

void menuSettingsUpdate() {
  lcd.home();
  lcd.print("<-- Updates");
  lcd.setCursor(0, 1);
  lcd.print("1: Short time update");
  lcd.setCursor(0, 2);
  lcd.print("2: Long time update");
  lcd.blink();
}

void menuSettingsUpdateShort() {
  lcd.home();
  lcd.print("<-- Updates");
  lcd.setCursor(0, 1);
  lcd.print("u");
  lcd.setCursor(0, 2);
  lcd.print(updateShort);
  lcd.setCursor(0, 3);
  lcd.print("d");
  lcd.noBlink();
  updateShort = updateShort - menuY;
  menuY = 0;
}

void menuSettingsUpdateLong() {
  lcd.home();
  lcd.print("<-- Updates");
  lcd.setCursor(0, 1);
  lcd.print("u");
  lcd.setCursor(0, 2);
  lcd.print(updateLong);
  lcd.setCursor(0, 3);
  lcd.print("d");
  lcd.noBlink;
  updateLong = updateLong - menuY;
  menuY = 0;
}

void menuSettingsWarning() {
  lcd.home();
  lcd.print("<--- Warning");
  lcd.setCursor(0, 1);
  lcd.print("1: One measurement");
  lcd.setCursor(0, 2);
  lcd.print("2: Short time");
  lcd.setCursor(0, 3);
  lcd.print("3: Long time");
  lcd.blink();
}

void menuSettingsWarningOne() {
  lcd.home();
  lcd.print("<--- Warning");
  lcd.setCursor(0, 1);
  lcd.print("u");
  lcd.setCursor(0, 2);
  lcd.print(warningOne);
  lcd.setCursor(0, 3);
  lcd.print("d");
  lcd.noBlink();
  warningOne = warningOne - menuY;
  menuY = 0;
}

void menuSettingsWarningShort() {
  lcd.home();
  lcd.print("<--- Warning");
  lcd.setCursor(0, 1);
  lcd.print("u");
  lcd.setCursor(0, 2);
  lcd.print(warningShort);
  lcd.setCursor(0, 3);
  lcd.print("d");
  lcd.noBlink();
  warningShort = warningShort - menuY;
  menuY = 0;
}

void menuSettingsWarningLong() {
  lcd.home();
  lcd.print("<--- Warning");
  lcd.setCursor(0, 1);
  lcd.print("u");
  lcd.setCursor(0, 2);
  lcd.print(warningLong);
  lcd.setCursor(0, 3);
  lcd.print("d");
  lcd.noBlink();
  warningLong = warningLong - menuY;
  menuY = 0;
}
