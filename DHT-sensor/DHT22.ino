// ----- DHT-avläsning
  #include "DHT.h"
  #define DHTPIN 8     // what pin we're connected to 
  #define DHTTYPE DHT22   // DHT 22  (AM2302) 
  DHT dht(DHTPIN, DHTTYPE);
  
  void setup() {
  Serial.begin(9600); 
  Serial.println("Startar!");
 
  dht.begin();
}

void loop() {
  // Två sekunder mellan mätningar.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Celsius
  float t = dht.readTemperature();
  // Fahrenheit
  float f = dht.readTemperature(true);
  
  // Must send in temp in Fahrenheit!
  float hi = dht.computeHeatIndex(f, h);

  Serial.print("Humidity: "); Serial.print(h); Serial.println(" %\t");
  Serial.print("Temperature: "); Serial.print(t); Serial.println(" *C ");
  Serial.print("Temperature: "); Serial.print(f); Serial.println(" *F\t");
  Serial.print("Heat index: "); Serial.print(hi); Serial.println(" *F");
}
