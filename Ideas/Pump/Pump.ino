//This program is used to turn G328 on and let it pump water for 2 minutes (once).
//Use MOS module (12 V), connect SIG to the pin 4.

void setup() {
  pinMode(4, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(4, HIGH);
  delay(120000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(4, LOW);

}

void loop() {
  delay(10000);

}
