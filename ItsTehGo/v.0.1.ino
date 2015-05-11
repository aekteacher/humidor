// http://www.network-science.de/ascii/ --- ASCIII :: rounded :: width=120

/*
MENUMENUMENUMENUMENU
    Copyright Giuseppe Di Cillo (www.coagula.org)
    Contact: dicillo@coagula.org

     http://www.arduino.cc/playground/uploads/Profiles/MenuBackend_1-4.zip

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
MENUMENUMENUMENUMENU
*/
//  Libraries
#include <MenuBackend.h>    //MenuBackend library - copyright by Alexander Brevig
#include <Wire.h>

#include <LCD03.h>        // LCD

#include <DHT.h>          // DHT sensor

#include <Time.h>        // Tiden
/*
 ____  _____ ____  _   _
|    \| ___ |  _ \| | | |
| | | | ____| | | | |_| |
|_|_|_|_____)_| |_|____/
*/
// Menu Buttons
const int buttonPinLeft = A0;      // pin for the Up button
const int buttonPinRight = A3;    // pin for the Down button
const int buttonPinEsc = A1;     // pin for the Esc button - up
const int buttonPinEnter = A2;   // pin for the Enter button - down

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
MenuItem menuItem1SubItem1 = MenuItem("Show Info"); 	// Show temp, hum, heat - Item1SubItem1
MenuItem menuItem1SubItem2 = MenuItem("Uptime");		// show uptime - Item1SubItem2
MenuItem menu1Item2 = MenuItem("Item2");					// Brightness
MenuItem menuItem2SubItem1 = MenuItem("Item2SubItem1");		// Edit Brightness
MenuItem menu1Item3 = MenuItem("Item3");					// Update
MenuItem menuItem3SubItem1 = MenuItem("Item3SubItem1");		// Edit short update
MenuItem menuItem3SubItem2 = MenuItem("Item3SubItem2");		// Edit long update
MenuItem menu1Item4 = MenuItem("Item4");					// Warning
MenuItem menuItem4SubItem1 = MenuItem("Item4SubItem1");		// Edit one warning
MenuItem menuItem4SubItem2 = MenuItem("Item4SubItem2");		// Edit short warning
MenuItem menuItem4SubItem3 = MenuItem("Item4SubItem3");		// Edit long warning

/*
 _     ___________
| |   /  __ \  _  \
| |   | /  \/ | | |
| |   | |   | | | |
| |___| \__/\ |/ /
\_____/\____/___/
*/

LCD03 lcd;  //this library is included in the Arduino IDE // Create new LCD03 instance

/*
______ _   _ _____             ___        _ _   _           _
|  _  \ | | |_   _|           / _ \      | (_) (_)         (_)
| | | | |_| | | |    ______  / /_\ \_   _| | __ _ ___ _ __  _ _ __   __ _
| | | |  _  | | |   |______| |  _  \ \ / / |/ _` / __| '_ \| | '_ \ / _` |
| |/ /| | | | | |            | | | |\ V /| | (_| \__ \ | | | | | | | (_| |
|___/ \_| |_/ \_/            \_| |_/ \_/ |_|\__,_|___/_| |_|_|_| |_|\__, |
                                                                     __/ |                                                                   |___/                                                                           (_____|
*/

#define DHTPIN 8         // what pin we're connected to 
#define DHTTYPE DHT22    // DHT 22  (AM2302) 
DHT dht(DHTPIN, DHTTYPE);

/*
 _   _       _
| | | |     | |
| | | | __ _| |_   _  ___  ___
| | | |/ _` | | | | |/ _ \/ __|
\ \_/ / (_| | | |_| |  __/\__ \
 \___/ \__,_|_|\__,_|\___||___/
*/
int setting[] = {75, 10, 20, 15, 25, 35};	// brightness, update short, update long, warning one, warning short, warning long
int v = 0;            // position i setting[]
int editValue = 0;          // korrigera värdet i setting [v]
boolean editValueState = false;     // om det får korrigeras

int d;      // dagar, timmar, minuter, sekunder
int H;
int m;
int s;

float h;      // Humidity
float t;      // Celsius
float f;      // Fahrenheit
float hi;     // Heatindex, fahrenheit, skrivs om till celsius raden under.
float hc;

void setup() {
  // ----- LCD-uppdatering
  lcd.begin(20, 4);

  // Clear the LCD
  lcd.backlight();
  lcd.clear();
  lcd.print("Welcome To Humidor");

  // ----- DHT-avläsning
  dht.begin();

  // ----- Menu
  pinMode(buttonPinLeft, INPUT);
  pinMode(buttonPinRight, INPUT);
  pinMode(buttonPinEnter, INPUT);
  pinMode(buttonPinEsc, INPUT);

  //configure menu
  menu.getRoot().add(menu1Item1);
  menu1Item1.addRight(menu1Item2).addRight(menu1Item3).addRight(menu1Item4);
  menu1Item1.add(menuItem1SubItem1).addRight(menuItem1SubItem2);
  menu1Item2.add(menuItem2SubItem1);
  menu1Item3.add(menuItem3SubItem1).addRight(menuItem3SubItem2);
  menu1Item4.add(menuItem4SubItem1).addRight(menuItem4SubItem2).addRight(menuItem4SubItem3);
  menu.toRoot();

  delay(2000);
}

void loop() {

  lcd.setBrightness(setting[0]);

  // ----- Tiden (Uptime - LCD)
  int d = day() - 1;      // korrigerad
  int H = hour();
  int m = minute();
  int s = second();

  // ----- DHT-avläsning
  h = dht.readHumidity();        // måste nog korrigeras med +4
  // Celsius
  t = dht.readTemperature();
  // Fahrenheit
  f = dht.readTemperature(true);
  // Heatindex, fahrenheit, skrivs om till celsius
  hc = (dht.computeHeatIndex(f, h) - 32) / 1.8;

  delay(250);   // tid för att läsa av, typ.

  readButtons();  //I splitted button reading and navigation in two procedures because
  navigateMenus();  //in some situations I want to use the button for other purpose (eg. to change some settings)
}

/*
 ____  _____ ____  _   _
|    \| ___ |  _ \| | | |
| | | | ____| | | | |_| |
|_|_|_|_____)_| |_|____/
*/
void menuChanged(MenuChangeEvent changed) {

  MenuItem newMenuItem = changed.to; //get the destination menu

  lcd.clear();
  lcd.home();
  lcd.print("Arduinodor!");
  lcd.setCursor(0, 1); //set the start position for lcd printing to the second row

  if (newMenuItem.getName() == menu.getRoot()) {
    lcd.print("Main Menu           ");
    lcd.setCursor(0, 2); lcd.print("Temperature: "); lcd.print(t); lcd.print("\x00B0"); lcd.print("C");
    lcd.setCursor(0, 3); lcd.print("Humidity: "); lcd.print(h); lcd.print(" %");
    editValueState = false;
  }
  else if (newMenuItem.getName() == "Item1") {
    lcd.print("Information         ");
  }
  else if (newMenuItem.getName() == "Show Info") {
    lcd.home();
    lcd.print("Information         ");
    lcd.setCursor(0, 1); lcd.print("Temperature: "); lcd.print(t); lcd.print("\x00B0"); lcd.print("C");
    lcd.setCursor(0, 2); lcd.print("Humidity:    "); lcd.print(h); lcd.print(" %");
    lcd.setCursor(0, 3); lcd.print("Heat:        "); lcd.print(hc); lcd.print("\x00B0"); lcd.print("C");
  }
  else if (newMenuItem.getName() == "Uptime") {
    lcd.home();
    lcd.print("Information         ");
    lcd.setCursor(0, 1);
    lcd.print("Uptime:           ");
    lcd.setCursor(0, 2);
    lcd.print(d); lcd.print(" d, "); lcd.print(H); lcd.print(" h, "); lcd.print(m); lcd.print(" min, "); lcd.print(s); lcd.print(" s.");
  }
  else if (newMenuItem.getName() == "Item2") {
    lcd.print("Brightness          ");
  }
  else if (newMenuItem.getName() == "Item2SubItem1") {
    v = 0;
    lcd.print("Brightness          ");
    lcd.setCursor(8, 2); lcd.print(setting[v]);
    lcd.setCursor(0, 3); lcd.print("Edit");
  }
  else if (newMenuItem.getName() == "Item3") {
    lcd.print("Updates             ");
  }
  else if (newMenuItem.getName() == "Item3SubItem1") {
    v = 1;
    lcd.print("Update TS Short Span");
    lcd.setCursor(8, 2); lcd.print(setting[v]);
    lcd.setCursor(0, 3); lcd.print("Edit");
  }
  else if (newMenuItem.getName() == "Item3SubItem2") {
    v = 2;
    lcd.print("Update TS Long Span ");
    lcd.setCursor(8, 2); lcd.print(setting[v]);
    lcd.setCursor(0, 3); lcd.print("Edit");
  }
  else if (newMenuItem.getName() == "Item4") {
    lcd.print("Warning             ");
  }
  else if (newMenuItem.getName() == "Item4SubItem1") {
    v = 3;
    lcd.print("Warning Single Data ");
    lcd.setCursor(8, 2); lcd.print(setting[v]);
    lcd.setCursor(0, 3); lcd.print("Edit");
  }
  else if (newMenuItem.getName() == "Item4SubItem2") {
    v = 4;
    lcd.print("Warning Short Span  ");
    lcd.setCursor(8, 2); lcd.print(setting[v]);
    lcd.setCursor(0, 3); lcd.print("Edit");
  }
  else if (newMenuItem.getName() == "Item4SubItem3") {
    v = 5;
    lcd.print("Warning Long Span   ");
    lcd.setCursor(8, 2); lcd.print(setting[v]);
    lcd.setCursor(0, 3); lcd.print("Edit");
  }
}

void menuUsed(MenuUseEvent used) {
  editValueState = true;
  lcd.setCursor(0, 0);
  lcd.print("Edit                  ");
  setting[v] = setting[v] + editValue;
  editValue = 0;
  lcd.setCursor(0, 2);
  lcd.print(" <-- : "); lcd.print(setting[v]); lcd.print(" : --> ");
  lastButtonPushed = 0;
  lcd.setCursor(0, 3);
}

void  readButtons() { //read buttons status
  int reading;
  int buttonEnterState = LOW;           // the current reading from the Enter input pin
  int buttonEscState = LOW;           // the current reading from the input pin
  int buttonLeftState = LOW;           // the current reading from the input pin
  int buttonRightState = LOW;           // the current reading from the input pin

  //Enter button - read the state of the switch into a local variable:
  reading = digitalRead(buttonPinEnter);

  // check to see if you just pressed the Enter button (i.e. the input went from LOW to HIGH),
  //  and you've waited long enough since the last press to ignore any noise:

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

  // check to see if you just pressed the Down button(i.e. the input went from LOW to HIGH),
  // and you've waited long enough since the last press to ignore any noise:

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

  // check to see if you just pressed the Right button(i.e. the input went from LOW to HIGH),
  // and you've waited long enough since the last press to ignore any noise:

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

  // check to see if you just pressed the Left button(i.e. the input went from LOW to HIGH),
  // and you've waited long enough since the last press to ignore any noise:

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
      if (editValueState) { //if its the last menu (no child) it will edit value and restart last menu
        editValue = 1;
        menu.use();
      } else { //otherwise, if menu has no child and has been pressed enter the current menu is used
        menu.moveRight();
      }
      break;
    case buttonPinLeft:
      if (editValueState) { //if its the last menu (no child) it will edit value and restart last menu
        editValue = -1;
        menu.use();
      } else { //otherwise, if menu has no child and has been pressed enter the current menu is used
        menu.moveLeft();
      }
      break;
  }
  lastButtonPushed = 0; //reset the lastButtonPushed variable
}
