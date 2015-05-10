/*
    Copyright Giuseppe Di Cillo (www.coagula.org)
    Contact: dicillo@coagula.org

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
IMPORTANT: to use the menubackend library by Alexander Brevig download it at http://www.arduino.cc/playground/uploads/Profiles/MenuBackend_1-4.zip and add the next code at line 195
	void toRoot() {
		setCurrent( &getRoot() );
	}
*/
#include <MenuBackend.h>    //MenuBackend library - copyright by Alexander Brevig
#include <Wire.h>
#include <LCD03.h>

// Create new LCD03 instance
LCD03 lcd;  //this library is included in the Arduino IDE

const int buttonPinLeft = 5;      // pin for the Up button
const int buttonPinRight = 2;    // pin for the Down button
const int buttonPinEsc = 3;     // pin for the Esc button - up
const int buttonPinEnter = 4;   // pin for the Enter button - down

int lastButtonPushed = 0;

int lastButtonEnterState = LOW;   // the previous reading from the Enter input pin
int lastButtonEscState = LOW;   // the previous reading from the Esc input pin
int lastButtonLeftState = LOW;   // the previous reading from the Left input pin
int lastButtonRightState = LOW;   // the previous reading from the Right input pin¨

long lastEnterDebounceTime = 0;  // the last time the output pin was toggled
long lastEscDebounceTime = 0;  // the last time the output pin was toggled
long lastLeftDebounceTime = 0;  // the last time the output pin was toggled
long lastRightDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 500;    // the debounce time

//Menu variables
MenuBackend menu = MenuBackend(menuUsed, menuChanged);
//initialize menuitems
  MenuItem menu1Item1 = MenuItem("Item1");					// Info
    MenuItem menuItem1SubItem1 = MenuItem("Item1SubItem1"); 	// Show temp, hum, heat
    MenuItem menuItem1SubItem2 = MenuItem("Item1SubItem2");		// show uptime
  MenuItem menu1Item2 = MenuItem("Item2");					// Brightness
    MenuItem menuItem2SubItem1 = MenuItem("Item2SubItem1");		// Edit Brightness
  MenuItem menu1Item3 = MenuItem("Item3");					// Update
    MenuItem menuItem3SubItem1 = MenuItem("Item3SubItem1");		// Edit short update
    MenuItem menuItem3SubItem2 = MenuItem("Item3SubItem2");		// Edit long update
  MenuItem menu1Item4 = MenuItem("Item4");					// Warning
    MenuItem menuItem4SubItem1 = MenuItem("Item4SubItem1");		// Edit one warning 
    MenuItem menuItem4SubItem2 = MenuItem("Item4SubItem2");		// Edit short warning
    MenuItem menuItem4SubItem3 = MenuItem("Item4SubItem3");		// Edit long warning

int setting[]={75, 10, 20, 15, 25, 35};	// brightness, update short, update long, warning one, warning short, warning long
int v = 0;
	
void setup()
{
  pinMode(buttonPinLeft, INPUT);
  pinMode(buttonPinRight, INPUT);
  pinMode(buttonPinEnter, INPUT);
  pinMode(buttonPinEsc, INPUT);

  lcd.begin(20, 4);

  // Clear the LCD
  lcd.backlight();
  lcd.clear();
  lcd.print("Welcome To Humidor");

  delay(2000);
  
  //configure menu
  menu.getRoot().add(menu1Item1);
  menu1Item1.addRight(menu1Item2).addRight(menu1Item3).addRight(menu1Item4);
  menu1Item1.add(menuItem1SubItem1).addRight(menuItem1SubItem2);
  menu1Item2.add(menuItem2SubItem1);
  menu1Item3.add(menuItem3SubItem1).addRight(menuItem3SubItem2);
  menu1Item4.add(menuItem4SubItem1).addRight(menuItem4SubItem2).addRight(menuItem4SubItem3);
  menu.toRoot();


}  // setup()...

void loop()
{
  readButtons();  //I splitted button reading and navigation in two procedures because
  navigateMenus();  //in some situations I want to use the button for other purpose (eg. to change some settings)
} //loop()...


void menuChanged(MenuChangeEvent changed) {

  MenuItem newMenuItem = changed.to; //get the destination menu

  lcd.clear();
  lcd.home();
  lcd.print("Arduinodor!");
  lcd.setCursor(0, 1); //set the start position for lcd printing to the second row

  if (newMenuItem.getName() == menu.getRoot()) {
    lcd.print("Main Menu       ");
	lcd.setCursor(0, 2);lcd.print("Temperature: 30");
	lcd.setCursor(0, 3);lcd.print("Humidity: 75 %");
  }
  else if (newMenuItem.getName() == "Item1") {
    lcd.print("Information     ");
  } 
  else if (newMenuItem.getName() == "Item1SubItem1") {
    lcd.home();lcd.print("Information      ");
	lcd.setCursor(0, 1);lcd.print("Temperature: 30 C");
	lcd.setCursor(0, 2);lcd.print("Humidity: 75 %");
	lcd.setCursor(0, 3);lcd.print("Heat:  25 C");
  }
  else if (newMenuItem.getName() == "Item1SubItem2") {
    lcd.home();lcd.print("Information      ");
	lcd.setCursor(0, 1);lcd.print("Uptime");
	lcd.setCursor(0, 2);lcd.print("1 d 2 h 3 min 54 s");
  } 
  else if (newMenuItem.getName() == "Item2") {
    lcd.print("Brightness      ");
  } 
  else if (newMenuItem.getName() == "Item2SubItem1") {
    lcd.print("Brightness    ");
	lcd.setCursor(8, 2);lcd.print("75");
	lcd.setCursor(0, 3);lcd.print("Edit");
	v = 0;
  } 
  else if (newMenuItem.getName() == "Item3") {
    lcd.print("Updates    ");
  }
  else if (newMenuItem.getName() == "Item3SubItem1") {
    lcd.print("Short Update");
	lcd.setCursor(8, 2);lcd.print("75");
	lcd.setCursor(0, 3);lcd.print("Edit");
	v = 1;
  } 
  else if (newMenuItem.getName() == "Item3SubItem2") {
    lcd.print("Long Update");
	lcd.setCursor(8, 2);lcd.print("75");
	lcd.setCursor(0, 3);lcd.print("Edit");
	v = 2;
  }
   else if (newMenuItem.getName() == "Item4") {
	lcd.print("Warning      ");
  }
  else if (newMenuItem.getName() == "Item4SubItem1") {
    lcd.print("One Measurement");
	lcd.setCursor(8, 2);lcd.print("10");
	lcd.setCursor(0, 3);lcd.print("Edit");
	v = 3;
  } 
  else if (newMenuItem.getName() == "Item4SubItem2") {
    lcd.print("Short Warn");
	lcd.setCursor(8, 2);lcd.print("75");
	lcd.setCursor(0, 3);lcd.print("Edit");
	v = 4;
  }
   else if (newMenuItem.getName() == "Item4SubItem3") {
    lcd.print("Warning");
	lcd.setCursor(8, 2);lcd.print("75");
	lcd.setCursor(0, 3);lcd.print("Edit");
	v = 5;
  }
}

void menuUsed(MenuUseEvent used) {
  lcd.setCursor(0, 0);
  lcd.print("Edit           ");
  lcd.setCursor(0, 2);
  lcd.print(" <-- : ");lcd.print(setting[v]);lcd.print(" : --> ");
  lcd.setCursor(0, 3);
  lcd.print("You used: ");lcd.print(used.item.getName());
  delay(3000);  //delay to allow message reading
  menu.toRoot();  //back to Main
}

void  readButtons() { //read buttons status
  int reading;
  int buttonEnterState = LOW;           // the current reading from the Enter input pin
  int buttonEscState = LOW;           // the current reading from the input pin
  int buttonLeftState = LOW;           // the current reading from the input pin
  int buttonRightState = LOW;           // the current reading from the input pin

  //Enter button - read the state of the switch into a local variable:
  reading = digitalRead(buttonPinEnter);

  // check to see if you just pressed the enter button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonEnterState) {
    // reset the debouncing timer
    lastEnterDebounceTime = millis();
  }

  if ((millis() - lastEnterDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    buttonEnterState = reading;
    lastEnterDebounceTime = millis();
  }

  // save the reading.  Next time through the loop, it'll be the lastButtonState:
  lastButtonEnterState = reading;

  //Esc button - read the state of the switch into a local variable:
  reading = digitalRead(buttonPinEsc);

  // check to see if you just pressed the Down button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonEscState) {
    // reset the debouncing timer
    lastEscDebounceTime = millis();
  }

  if ((millis() - lastEscDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    buttonEscState = reading;
    lastEscDebounceTime = millis();
  }

  // save the reading.  Next time through the loop, it'll be the lastButtonState:
  lastButtonEscState = reading;

  //Down button - read the state of the switch into a local variable:
  reading = digitalRead(buttonPinRight);

  // check to see if you just pressed the Down button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonRightState) {
    // reset the debouncing timer
    lastRightDebounceTime = millis();
  }

  if ((millis() - lastRightDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    buttonRightState = reading;
    lastRightDebounceTime = millis();
  }

  // save the reading.  Next time through the loop, it'll be the lastButtonState:
  lastButtonRightState = reading;

  //Up button - read the state of the switch into a local variable:
  reading = digitalRead(buttonPinLeft);

  // check to see if you just pressed the Down button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonLeftState) {
    // reset the debouncing timer
    lastLeftDebounceTime = millis();
  }

  if ((millis() - lastLeftDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    buttonLeftState = reading;
    lastLeftDebounceTime = millis();;
  }

  // save the reading.  Next time through the loop, it'll be the lastButtonState:
  lastButtonLeftState = reading;

  //records which button has been pressed
  if (buttonEnterState == HIGH) {
    lastButtonPushed = buttonPinEnter;

  } else if (buttonEscState == HIGH) {
    lastButtonPushed = buttonPinEsc;

  } else if (buttonRightState == HIGH) {
    lastButtonPushed = buttonPinRight;

  } else if (buttonLeftState == HIGH) {
    lastButtonPushed = buttonPinLeft;

  } else {
    lastButtonPushed = 0;
  }
}

void navigateMenus() {
  MenuItem currentMenu = menu.getCurrent();

  switch (lastButtonPushed) {
    case buttonPinEnter:
      if (!(currentMenu.moveDown())) { //if the current menu has a child and has been pressed enter then menu navigate to item below
        menu.use();
      } else { //otherwise, if menu has no child and has been pressed enter the current menu is used
        menu.moveDown();
      }
      break;
    case buttonPinEsc:
      menu.toRoot();  //back to main
      break;
    case buttonPinRight:
      menu.moveRight();
      break;
    case buttonPinLeft:
      menu.moveLeft();
      break;
  }
  lastButtonPushed = 0; //reset the lastButtonPushed variable
}
