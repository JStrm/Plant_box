//This program is used to check CO2 concentration in the air, temperature, pressure and humidity every ten minutes
//and to turn a pump on twice a day.
//MH-Z14 needs to be connected to 5V and GND (V+, V-) and to pins 7 (T) and 8 (R).
//BME280 needs to be connected to 5V, GND, A5 and A4 (VIN, GND, SCL, SDA).
//G328 needs to be supplied by 12 V. Use MOS module, connect SIG to 4 (VCC and GND to 5V and GND).
//MH-Z14 may need some time to heat up.

#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

byte pumpTime = 0;

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

SoftwareSerial Serial2(7, 8);
byte addArray[] = {
0xFF, 0x01, 0x86,
0x00, 0x00, 0x00,
0x00, 0x00, 0x79
};
char dataValue[9];
String dataString = "";

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  Serial.println("BME280 test");
  unsigned status;
  status = bme.begin(0x76);
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
  }
  
  Serial.println("Heating up.");
  delay(10000);
  Serial.println("Running MH-Z14.");
  Serial.println();
}

void loop() {
  unsigned long start = millis();
  printCO2();
  printTPH();
  checkPump();
  if (millis() > start) {
    start = millis() - start;
    delay(600000 - start);
  }
  else {
    delay(600000);
  }
}

void checkPump() {
  pumpTime = pumpTime + 1;
  if (pumpTime == 72) {
    pumpTime = 0;
    digitalWrite(4, HIGH);
    delay(12000);
    digitalWrite(4, LOW);
  }
}

void printCO2() {
  int co2 = 0;
  Serial2.write(addArray, 9);  
  Serial2.readBytes(dataValue, 9);
  co2 = (256*(int)dataValue[2])+(int)dataValue[3];
  Serial.print("CO2 = "); Serial.print(co2); Serial.println(" ppm");
}

void printTPH() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}
