#include <SoftwareSerial.h>
#include <Wire.h>
// #include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <SD.h>

// Pins
const int MHZ14_T_pin       = 7;
const int MHZ14_R_pin       = 8;
const int G328_MOS_SIG_pin  = 4;
const int chipSelectSD      = 10;

// Declare bme sensor and CO2 serial
Adafruit_BME280 BME;
SoftwareSerial SerialCO2(MHZ14_T_pin, MHZ14_R_pin);

// Pressure at sea level in HPa
const float seaLevelPressure = 1013.25;


void setup() {
  Serial.begin(9600);

  // Begin software serial for MHZ14
  SerialCO2.begin(9600);
  while(!SerialCO2)
    ;

  // Begin temp and pres sensor
  BME.begin(0x76);

  // Heat up CO2 sensor
  delay(10000);

  // Initialize SD card
  if (!SD.begin(chipSelectSD)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");

  //First line into log
  writeLineToFile("log.txt", "time(ms),tempeature(°C),pressure(hPa),humidity(%),altitude(m),CO2(ppm)");
}

void loop() {
  String dataString = "";

  dataString += millis();
  dataString += ',';

  dataString += BME.readTemperature();
  dataString += ',';

  dataString += (BME.readPressure() / 100.0F);
  dataString += ',';

  dataString += BME.readHumidity();
  dataString += ',';

  dataString += BME.readAltitude(seaLevelPressure);
  dataString += ',';

  dataString += getCO2();
  dataString += ',';
  
  writeLineToFile("log.txt", dataString);
  delay(100000);
}

void writeLineToFile(String fileName, String string){
  File file = SD.open(fileName, FILE_WRITE);

  if (file) {
    file.println(string);
    file.close();
    // print to the serial port too:
    Serial.println(string);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}

const byte readCO2[] = {
  0xFF, 0x01, 0x86,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x79
};

// Returns ppm of CO2
int getCO2(){
  char dataValue[9];
  int co2 = -1;

  SerialCO2.write(readCO2, 9);  
  SerialCO2.readBytes(dataValue, 9);
  
  co2 = (256 * (int)dataValue[2]) + (int)dataValue[3];
  return co2;
}

