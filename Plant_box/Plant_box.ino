#include <Wire.h>
// #include <Adafruit_BME280.h>
#include <dhtnew.h>
#include <SPI.h>
#include <SD.h>

const byte mhz14PwmPin = 9;  // PWM pin for CO2 sensor
const byte csSdPin = 4;      // Chip select for SD card reader
const byte dhtPin = 7;      // Chip select for SD card reader
const byte pumpPin = 3;      // Pump pin
const byte writeLedPin = 5;
const byte errorLedPin = 6;
const byte photoRezistorPin = A7;

const float seaLevelPressure = 1013.25;
const int co2ReadingSpan = 5000;
const char filename[8] = "log.txt";
// const int maxSdFailCount = 14 * 3;  // 14 writes, so if stuck for 5m stop

const unsigned long co2HeatUpTime = 180000;  // 3m in ms
const unsigned long readingPeriod = 10000;   // 10s in ms
const unsigned long pumpPause = 86400000;    // 24h in ms
const unsigned long pumpTime = 60000;        // 1 min in ms


DHTNEW dhtSensor(dhtPin);

void setup() {
  pinMode(pumpPin, OUTPUT);
  pinMode(writeLedPin, OUTPUT);
  pinMode(errorLedPin, OUTPUT);
  pinMode(photoRezistorPin, INPUT);

  // BME.begin(0x76);

  if (!SD.begin(csSdPin)) {
    somethingIsWrong();
  }

  digitalWrite(writeLedPin, HIGH);
  // delay(co2HeatUpTime);
  digitalWrite(writeLedPin, LOW);

  writeToFile(F("\ntime(s),temperature(°C),pressure(hPa),humidity(%),CO2PWM(ppm),light level\n"));
}

unsigned long nextPumpTime = 10000;
unsigned long nextReading = 0;
bool pumpOn = false;
const char separator[3] = ", ";

void loop() {
  if (nextReading < millis()) {
    digitalWrite(writeLedPin, HIGH);

    dhtSensor.read();

    writeToFile(String(millis() / 1000));  // time
    writeToFile(separator);
    writeToFile(String(dhtSensor.getTemperature(), 2));  // temp
    writeToFile(separator);
    writeToFile("0");  // pressure
    writeToFile(separator);
    writeToFile(String(dhtSensor.getHumidity(), 2));  // hum
    writeToFile(separator);
    writeToFile(String(pwmCo2Concentration()));  //co2
    writeToFile(separator);
    writeToFile(String(analogRead(photoRezistorPin)));  //light
    writeToFile(separator);
    writeToFile(F("\n"));

    digitalWrite(writeLedPin, LOW);
    delay(100);

    nextReading += readingPeriod;
    if (nextReading < millis()) {
      nextReading = millis() + readingPeriod;
    }
  }


  if ((nextPumpTime < millis()) && !pumpOn) {
    digitalWrite(pumpPin, HIGH);
    nextPumpTime += pumpTime;
    pumpOn = true;
  }

  if ((nextPumpTime < millis()) && pumpOn) {
    digitalWrite(pumpPin, LOW);
    nextPumpTime += pumpPause;
    pumpOn = false;
  }
}

void writeToFile(String string) {
  File file = SD.open(filename, FILE_WRITE);

  bool written = false;

  while (!written) {
    if (file) {
      file.print(string);
      file.close();
      written = true;
    } else {
      somethingIsWrong();
    }
    file = SD.open(filename, FILE_WRITE);
  }
}

int pwmCo2Concentration() {
  unsigned long tH = pulseInLong(mhz14PwmPin, HIGH, 2000000L);
  unsigned long tL = pulseInLong(mhz14PwmPin, LOW, 2000000L);
  unsigned long ppm = co2ReadingSpan * (tH - 2) / (tH + tL - 4);

  return int(ppm);
}

void somethingIsWrong() {
  while (!SD.begin(csSdPin)) {
    digitalWrite(errorLedPin, HIGH);
    delay(100);
    digitalWrite(errorLedPin, LOW);
    delay(50);
  }
  digitalWrite(errorLedPin, LOW);
  delay(500);
}
