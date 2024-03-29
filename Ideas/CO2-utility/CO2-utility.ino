#include <SoftwareSerial.h>

SoftwareSerial Serial2(7, 8);

byte readConcentrationArray[] = {
  0xFF, 0x01, 0x86,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x79
};

byte zeroPointCalibrationArray[] = {
  0xFF, 0x01, 0x87,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x78
};

// For 2000 ppm
byte spanPointCalibrationArray[] = {
  0xFF, 0x01, 0x88,
  0x07, 0xD0, 0x00,
  0x00, 0x00, 0xA0
};

byte turnOnSelfCalibrationArray[] = {
  0xFF, 0x01, 0x79,
  0xA0, 0x00, 0x00,
  0x00, 0x00, 0xE6
};

byte turnOffSelfCalibrationArray[] = {
  0xFF, 0x01, 0x79,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x86
};

char dataValue[9];

int input = 0;

void setup() {
  Serial2.begin(9600);
  Serial.begin(9600);
  delay(2000);
  Serial.println("What do you want to do?");
  Serial.println("1 -> Set zero point.");
  Serial.println("2 -> Set span point for 2000 ppm.");
  Serial.println("3 -> Turn off self calibration.");
  Serial.println("4 -> Turn on self calibration.");
  Serial.println("5 -> See readings.");
  Serial.println("6 -> Set span point for custom concentration.");
}

void loop() {
  if (Serial.available() > 0)
  {
    input = Serial.read() - 48;
    switch (input) {
      case 1:
        Serial2.write(zeroPointCalibrationArray, 9);
        Serial.println("Zero point set.");
        break;
      case 2:
        Serial2.write(spanPointCalibrationArray, 9);
        Serial.println("Span point set.");
        break;
      case 3:
        Serial2.write(turnOffSelfCalibrationArray, 9);
        Serial.println("Auto calibration off.");
        break;
      case 4:
        Serial2.write(turnOnSelfCalibrationArray, 9);
        Serial.println("Auto calibration on.");
        break;
      case 5:
        Serial.println("Input anything to break.");

        while (Serial.available() == 0) {
          int co2 = 0;
          Serial2.write(readConcentrationArray, 9);
          Serial2.readBytes(dataValue, 9);
          co2 = (256 * (int)dataValue[2]) + (int)dataValue[3];
          Serial.print("CO2: ");
          Serial.print(co2);
          Serial.println(" ppm");
          delay(2000);
        }
        break;
      case 6:
        Serial.println("Input the CO2 concentration in ppm.");
        while (Serial.available() == 0) {}
        int co2 = Serial.readString().toInt();
        byte customSpanPointCalibrationArray[] = {
          0xFF, 0x01, 0x88,
          0x07, 0xD0, 0x00,
          0x00, 0x00, 0xA0
        };
        customSpanPointCalibrationArray[3] = (byte)(co2/256);
        customSpanPointCalibrationArray[4] = (byte)(co2%256);
        customSpanPointCalibrationArray[8] = 0x77 - customSpanPointCalibrationArray[3] - customSpanPointCalibrationArray[4];
        Serial2.write(customSpanPointCalibrationArray, 9);
        Serial.println("Span point set.");
        break;
    }
    Serial.println("What do you want to do?");
    Serial.println("1 -> Set zero point.");
    Serial.println("2 -> Set span point for 2000 ppm.");
    Serial.println("3 -> Turn off self calibration.");
    Serial.println("4 -> Turn on self calibration.");
    Serial.println("5 -> See readings.");
    Serial.println("6 -> Set span point for custom concentration.");
  }
}
