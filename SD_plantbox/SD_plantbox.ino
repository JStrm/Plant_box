#include <SoftwareSerial.h>
#include <Wire.h>
// #include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <SD.h>

// Pins
const byte MHZ14_T_pin       = 7;   // CO2 sensor
const byte MHZ14_R_pin       = 8;   // CO2 sensor
const byte G328_MOS_SIG_pin  = 4;   // Pump pin
const byte chipSelectSD      = 10;  // Chip select for SD card reader
const byte MHZ14_pwm_pin     = 2;   // PWM pin for CO2 sensor
                                    // BME uses hard-coded I2C pins

// Declare bme sensor and CO2 serial
Adafruit_BME280 BME;
SoftwareSerial SerialCO2(MHZ14_T_pin, MHZ14_R_pin);

// Pressure at sea level in HPa
const float seaLevelPressure = 1013.25;

// Reading span of CO2 sensor
const int span = 5000;

void setup() {
  pinMode(G328_MOS_SIG_pin, OUTPUT);

  //Serial.begin(9600);

  // Begin software serial for MHZ14
  SerialCO2.begin(9600);
  while(!SerialCO2)
    ;

  // Begin temp and pressure sensor
  BME.begin(0x76);

  // Heat up CO2 sensor
  //Serial.println("Heating up CO2 sensor (3m)");
  delay((unsigned long)180000);

  // Initialize SD card
  if (!SD.begin(chipSelectSD)) {
    //Serial.println("Card failed, or not present");
    while (1);
  }
  //Serial.println("card initialized.");

  //First line into log
  writeLineToFile(F("log.txt"), F("time(s),tempeature(°C),pressure(hPa),humidity(%),altitude(m),CO2(ppm),CO2PWM(ppm)"));
}

unsigned long nextPumpTime  = 0;
unsigned long nextReading   = 0;
bool pumpOn = false;
String dataString = "";

void loop() {
  if (nextReading < millis())
  {
    dataString = "";

    dataString += (millis() / (unsigned long) 1000);
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

    dataString += getCO2PWM();
    //dataString += ',';
    
    writeLineToFile(F("log.txt"), dataString);
    
    nextReading = nextReading + (unsigned long) 10 * 1000;
  }
 

  if ((nextPumpTime < millis()) && (!pumpOn)) {
    digitalWrite(G328_MOS_SIG_pin, HIGH);
    nextPumpTime = nextPumpTime + (unsigned long) 60 * 1000;
    pumpOn = true;
  }
  
  if ((nextPumpTime < millis()) && pumpOn) {
    digitalWrite(G328_MOS_SIG_pin, LOW);
    nextPumpTime = nextPumpTime + (unsigned long) 48 * 60 * 60 * 1000 - (unsigned long) 60 * 1000;
    pumpOn = false;
  }
}

void writeLineToFile(String fileName, String string){
  File file = SD.open(fileName, FILE_WRITE);

  if (file) {
    file.println(string);
    file.close();
    // print to the serial port too:
    //Serial.println(string);
  }
  // if the file isn't open, pop up an error:
  /*else {
    Serial.println("error opening log.txt");
  }*/
}

const byte readCO2[] = {
  0xFF, 0x01, 0x86,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x79
};

// Returns ppm of CO2
int getCO2(){
  char dataValue[9];
  int co2;

  SerialCO2.write(readCO2, 9);  
  SerialCO2.readBytes(dataValue, 9);
  
  co2 = (256 * (int)dataValue[2]) + (int)dataValue[3];
  return co2;
}

int getCO2PWM(){
  unsigned long th, tl, ppm_pwm = 0;
  do {
    th = pulseIn(MHZ14_pwm_pin, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm_pwm = span * (th - 2) / (th + tl - 4);
  } while (th == 0);
  
  return ppm_pwm;
}

