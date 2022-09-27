//This program is used to check CO2 concentration in the air every minute.
//MH-Z14 needs to be connected to 5V and GND (V+, V-) and to pins 7 (T) and 8 (R).
//It may need some time at the beginning to heat up.

#include <SoftwareSerial.h>

SoftwareSerial Serial2(7, 8);

byte addArray[] = {
0xFF, 0x01, 0x86,
0x00, 0x00, 0x00,
0x00, 0x00, 0x79
};

char dataValue[9];
String dataString = "";

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial.println("Getting ready.");
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
  Serial.println("Running MH-Z14");

}

void loop() {
  int co2 = 0;
  
  Serial2.write(addArray, 9);  
  Serial2.readBytes(dataValue, 9);
  co2 = (256*(int)dataValue[2])+(int)dataValue[3];
  Serial.print("CO2: "); Serial.print(co2); Serial.println(" ppm");
}
