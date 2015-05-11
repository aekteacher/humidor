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

void setup() {
  // Initialise a 20x4 LCD
  lcd.begin(20, 4);

  pinMode(BUTTON_PIN_UP, INPUT);
  pinMode(BUTTON_PIN_DOWN, INPUT);

  // Clear the LCD
  lcd.backlight();
  lcd.clear();
}

void loop() {
  buttonStateUp = digitalRead(BUTTON_PIN_UP);
  buttonStateDown = digitalRead(BUTTON_PIN_DOWN);
  buttonStateLeft = digitalRead(BUTTON_PIN_LEFT);
  buttonStateRight = digitalRead(BUTTON_PIN_RIGHT);

  delay(500);

  if (buttonStateUp == LOW)  {
    lcd.home();
    lcd.print("UPUPUPUPUPUPUPUP");
  }
  if (buttonStateDown == LOW)  {
    lcd.home();
    lcd.print("DOWNDOWNDOWNDOWN");
  }
  if (buttonStateLeft == LOW)  {
    lcd.home();
    lcd.print("LEFTLEFTLEFTLEFT");
  }
  if (buttonStateRight == LOW)  {
    lcd.home();
    lcd.print("RIGHTRIGHTRIGHTR");
  }
}
