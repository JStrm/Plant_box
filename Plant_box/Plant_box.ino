#include <Wire.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <SD.h>

const byte mhz14PwmPin = 10;  // PWM pin for CO2 sensor
const byte csSdPin = 4;       // Chip select for SD card reader
const byte pumpPin = 6;       // Pump pin
const byte ledPin = 9;        // Pump pin

const float seaLevelPressure = 1013.25;
const int co2ReadingSpan = 5000;
const char filename[8] = "log.txt";
const byte maxSdFailCount = 100;

Adafruit_BME280 BME;

void setup() {
  pinMode(pumpPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  BME.begin(0x76);

  digitalWrite(ledPin, HIGH);
  delay((unsigned long)180000);
  digitalWrite(ledPin, LOW);

  if (!SD.begin(csSdPin)) {
    somethingIsWrong();
  }

  writeLineToFile(F("time(s),temperature(°C),pressure(hPa),humidity(%),altitude(m),CO2PWM(ppm),"));
}

unsigned long nextPumpTime = 0;
unsigned long nextReading = 0;
bool pumpOn = false;
const char separator[3] = ", ";

void loop() {
  if (nextReading < millis()) {
    writeToFile(String(millis() / 10000));
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
    writeLineToFile(separator);
  }


  if ((nextPumpTime < millis()) && !pumpOn) {
    digitalWrite(pumpPin, HIGH);
    nextPumpTime += (unsigned long)60000;  // 1 min in ms
    pumpOn = true;
  }

  if ((nextPumpTime < millis()) && pumpOn) {
    digitalWrite(pumpPin, LOW);
    nextPumpTime += (unsigned long)86400000;  // 24h in ms
    pumpOn = false;
  }
}

byte sdFailCount = 0;
void writeLineToFile(String string) {
  File file = SD.open(filename, FILE_WRITE);

  if (file) {
    file.println(string);
    file.close();
  } else {
    sdFailCount++;
    if (sdFailCount > maxSdFailCount) {
      somethingIsWrong();
    }
  }
}

void writeToFile(String string) {
  File file = SD.open(filename, FILE_WRITE);

  if (file) {
    file.print(string);
    file.close();
  } else {
    sdFailCount++;
    if (sdFailCount > maxSdFailCount) {
      somethingIsWrong();
    }
  }
}

int pwmCo2Concentration() {
  unsigned long th, tl, ppm_pwm = 0;
  do {
    th = pulseIn(mhz14PwmPin, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm_pwm = co2ReadingSpan * (th - 2) / (th + tl - 4);
  } while (th == 0);

  return ppm_pwm;
}

void somethingIsWrong() {
  while (true) {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}
