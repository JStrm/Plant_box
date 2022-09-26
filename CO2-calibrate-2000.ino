#include <SoftwareSerial.h>

SoftwareSerial Serial2(7, 8);

byte fullArray[] = {
0xFF, 0x01, 0x88,
0x07, 0xD0, 0x00,
0x00, 0x00, 0xA0
};

byte addArray[] = {
0xFF, 0x01, 0x86,
0x00, 0x00, 0x00,
0x00, 0x00, 0x79
};

char dataValue[9];
String dataString = "";

void setup() {
  Serial2.begin(9600);
  Serial.begin(9600);
  delay(2000);
  Serial2.write(fullArray, 9);
  Serial.println("Calibration done.");
  delay(10000);
}

void loop() {
  int co2 = 0;
  Serial2.write(addArray, 9);  
  Serial2.readBytes(dataValue, 9);
  co2 = (256*(int)dataValue[2])+(int)dataValue[3];
  Serial.print("CO2: "); Serial.print(co2); Serial.println(" ppm");
  delay(2000);
}
