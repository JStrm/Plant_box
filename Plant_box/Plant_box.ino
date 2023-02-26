#include <Wire.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <SD.h>

const byte mhz14PwmPin = 9;  // PWM pin for CO2 sensor
const byte csSdPin = 4;      // Chip select for SD card reader
const byte pumpPin = 6;      // Pump pin
const byte writeLedPin = 5;       
const byte errorLedPin = 6;       
const byte photoRezistorPin = A7;

const float seaLevelPressure = 1013.25;
const int co2ReadingSpan = 5000;
const char filename[8] = "log.txt";
const int maxSdFailCount = 14 * 3;  // 14 writes, so if stuck for 5m stop

const unsigned long co2HeatUpTime = 180000;  // 3m in ms
const unsigned long readingPeriod = 10000;   // 10s in ms
const unsigned long pumpPause = 86400000;    // 24h in ms
const unsigned long pumpTime = 60000;        // 1 min in ms


Adafruit_BME280 BME;

void setup() {
  pinMode(pumpPin, OUTPUT);
  pinMode(writeLedPin, OUTPUT);
  pinMode(errorLedPin, OUTPUT);
  pinMode(photoRezistorPin, INPUT);

  BME.begin(0x76);

  if (!SD.begin(csSdPin)) {
    somethingIsWrong();
  }

  digitalWrite(writeLedPin, HIGH);
  // delay(co2HeatUpTime);
  digitalWrite(writeLedPin, LOW);

  writeToFile(F("\ntime(s),temperature(°C),pressure(hPa),humidity(%),altitude(m),CO2PWM(ppm),light level\n"));
}

unsigned long nextPumpTime = 10000;
unsigned long nextReading = 0;
bool pumpOn = false;
const char separator[3] = ", ";

void loop() {
  if (nextReading < millis()) {
    digitalWrite(writeLedPin, HIGH);

    writeToFile(String(millis() / 1000));
    writeToFile(separator);
    writeToFile(String(BME.readTemperature()));
    writeToFile(separator);
    writeToFile(String(BME.readPressure() / 100.0F));
    writeToFile(separator);
    writeToFile(String(BME.readHumidity()));
    writeToFile(separator);
    writeToFile(String(BME.readAltitude(seaLevelPressure)));
    writeToFile(separator);
    writeToFile(String(pwmCo2Concentration()));
    writeToFile(separator);
    writeToFile(String(analogRead(photoRezistorPin)));
    writeToFile(separator);
    writeToFile(F("\n"));

    digitalWrite(writeLedPin, LOW);

    nextReading += readingPeriod;
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

int sdFailCountInRow = 0;
void writeToFile(String string) {
  File file = SD.open(filename, FILE_WRITE);

  if (file) {
    file.print(string);
    file.close();
    sdFailCountInRow = 0;
  } else {
    sdFailCountInRow++;
    if (sdFailCountInRow > maxSdFailCount) {
      somethingIsWrong();
    }
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
  delay(500);
}
