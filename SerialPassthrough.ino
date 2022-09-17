// SerialPassthrogh for Stack-chan SCServo Model

#include <M5Stack.h>

void setup() {
  // Initialize M5Stack
  M5.begin();
  M5.Power.begin();
  // Initialize SerialPort
//Serial.begin(115200); // for PC (USB-Serial  already opened in M5.beguin)
  Serial2.begin(1000000); // for SCServo (Stack-chan bord include SignalConverter)
}
void loop() {
  // PC to SCServo
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial2.write(inByte);
    Serial2.flush(); // wait send
    Serial2.read(); // flash call back
  }
  // SCServo to PC
  if (Serial2.available()) {
    int inByte = Serial2.read();
    Serial.write(inByte);
    Serial.flush(); // wait send
  }
  M5.update();
}
