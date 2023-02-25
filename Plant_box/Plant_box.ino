#include <Wire.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <SD.h>

const byte MHZ14_pwm_pin = 10;      // PWM pin for CO2 sensor
const byte chip_select_SD_pin = 4;  // Chip select for SD card reader
const byte pump_pin = 6;            // Pump pin
const byte LED_pin = 9;             // Pump pin

const float sea_level_pressure = 1013.25;
const int co2_reading_span = 5000;
const char filename[8] = "log.txt";
const byte max_sd_fail_count = 100;

Adafruit_BME280 BME;

void setup() {
  pinMode(pump_pin, OUTPUT);
  pinMode(LED_pin, OUTPUT);

  Serial.begin(9600);

  BME.begin(0x76);

  Serial.println("Heating up CO2 sensor (3m)");
  delay((unsigned long)180000);

  if (!SD.begin(chip_select_SD_pin)) {
    something_is_wrong();
  }

  write_line_to_file(F("time(s),temperature(°C),pressure(hPa),humidity(%),altitude(m),CO2PWM(ppm),"));
}

unsigned long next_pump_time = 0;
unsigned long next_reading = 0;
bool pump_on = false;
const char separator[3] = ", ";

void loop() {
  if (next_reading < millis()) {
    write_to_file(String(millis() / 10000));
    write_to_file(separator);
    write_to_file(String(BME.readTemperature()));
    write_to_file(separator);
    write_to_file(String(BME.readPressure() / 100.0F));
    write_to_file(separator);
    write_to_file(String(BME.readHumidity()));
    write_to_file(separator);
    write_to_file(String(BME.readAltitude(sea_level_pressure)));
    write_to_file(separator);
    write_to_file(String(pwm_co2_concentration()));
    write_line_to_file(separator);
  }


  if ((next_pump_time < millis()) && !pump_on) {
    digitalWrite(pump_pin, HIGH);
    next_pump_time += (unsigned long)60000; // 1 min in ms
    pump_on = true;
  }

  if ((next_pump_time < millis()) && pump_on) {
    digitalWrite(pump_pin, LOW);
    next_pump_time += (unsigned long)86400000; // 24h in ms
    pump_on = false;
  }
}

byte sd_fail_count = 0;
void write_line_to_file(String string){
  File file = SD.open(filename, FILE_WRITE);

  if (file) {
    file.println(string);
    file.close();
  } else {
    sd_fail_count++;
    if (sd_fail_count > max_sd_fail_count){
      something_is_wrong();
    }
  }
}

void write_to_file(String string){
  File file = SD.open(filename, FILE_WRITE);

  if (file) {
    file.print(string);
    file.close();
  } else {
    sd_fail_count++;
    if (sd_fail_count > max_sd_fail_count){
      something_is_wrong();
    }
  }
}

int pwm_co2_concentration(){
  unsigned long th, tl, ppm_pwm = 0;
  do {
    th = pulseIn(MHZ14_pwm_pin, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm_pwm = co2_reading_span * (th - 2) / (th + tl - 4);
  } while (th == 0);
  
  return ppm_pwm;
}

void something_is_wrong(){
  while (true) {
      digitalWrite(LED_pin, HIGH);
      delay(100);
      digitalWrite(LED_pin, LOW);
      delay(100);
    }
}
