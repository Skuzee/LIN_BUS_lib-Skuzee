// Pro Micro reads the LIN data, and monitors LIN buttons and dash buttons. It is located in the dash.
// It sends the commands over a serial connection to another controller.
// The second controller is a Digispark and is in the battery box of the car and actuates the relays.

#include <avr/sleep.h>
#include <LIN_BUS_lib-Skuzee.h>

// define pins for LIN transceiver communication
#define LIN_TX_PIN 1
#define LIN_RX_PIN 0
#define CS_PIN 2

LINtransceiver myLIN(LIN_TX_PIN, LIN_RX_PIN, 9600, CS_PIN);

void setup() {
  Serial.begin(115200); // Serial is USB serial!
}


void loop() {
  if (millis() - myLIN.getTimestampOfLastByte() >= 30000) { // if it has been 10 seconds since the last LIN data signal...
    arduinoToSleep(); // Put the arduino to sleep to save power.
  }

  if (myLIN.read()) {
    printData(myLIN.getData());
  }

}

void printData(LIN_Data_t LIN_Data) {
  for (int i = 0; i < 11; i++) {
    switch (i) {
      case SYNC:
        Serial.println("");
        Serial.print("Sync:");
        Serial.print(LIN_Data.sync, HEX);
        Serial.print(",");
        break;
      case PID:
        Serial.print("PID:");
        Serial.print(LIN_Data.pid, HEX);
        Serial.print(",ID:");
        Serial.print(LIN_Data.id, HEX);
        Serial.print(",Data:");
        break;
      case DATA:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        Serial.print(LIN_Data.rawData[i], HEX);
        Serial.print(",");
        break;
      case CHECKSUM:
        Serial.print("CHECKSUM:");
        Serial.print(LIN_Data.rawData[i], HEX);
        break;
      default:
        Serial.println("ERROR");
    }
  }
}

void arduinoToSleep() { // puts arduino to sleep if there is no communication.
  Serial.println("");
  Serial.println("Good Night!");
  delay(1000);
  Serial.flush();
  Serial.end();
  Serial1.flush();
  Serial1.end();
  sleep_enable();
  pinMode(LIN_RX_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(LIN_RX_PIN), arduinoWakeUp, FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sei();
  sleep_cpu();
  Serial.println("Good Morning!");
}

void arduinoWakeUp() {
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(LIN_RX_PIN));
  myLIN.initLIN();
  Serial.println("Good Morning!");
}
