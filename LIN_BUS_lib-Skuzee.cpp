// Includes
#include "Arduino.h"
#include "LIN_BUS_lib-Skuzee.h"


LINtransceiver::LINtransceiver(uint8_t _TX_Pin, uint8_t _RX_Pin, uint16_t _Baud, uint8_t _CS_Pin)
{
	LIN_TX_Pin = _TX_Pin;
	LIN_RX_Pin = _RX_Pin;
	LIN_Baud = _Baud * 1UL;
	CS_Pin = _CS_Pin;
	initLIN();
}

LINtransceiver::LINtransceiver(uint8_t _TX_Pin, uint8_t _RX_Pin, uint16_t _Baud, uint8_t _CS_Pin, bool _legacyMode)
{
	LIN_TX_Pin = _TX_Pin;
	LIN_RX_Pin = _RX_Pin;
	LIN_Baud = _Baud * 1UL;
	CS_Pin = _CS_Pin;
	legacyMode = _legacyMode;
	initLIN();
}

LINtransceiver::initLIN(void) {
  Serial1.begin(LIN_Baud, SERIAL_8N1); // Serial 1 is the Tx/Rx pins that make a serial connection to the LIN transciever.
  
  pinMode(CS_Pin, OUTPUT); // Chip Select / Enable Pin
  digitalWrite(CS_Pin, LOW); // CS_Pin HIGH = Ready Mode, LOW = Sleep mode. (Will still wake on LIN falling edge!)
  pinMode(LIN_TX_Pin, OUTPUT);
  digitalWrite(LIN_TX_Pin, LOW); // LIN_TX_Pin LOW = TXoff Mode, HIGH = Operation Mode. // If you want the LIN transceiver to transmit then this pin must be high when doing so.
}

LINtransceiver::read(void) {
	// waitUntilNextMessage: waits/reads until SYNC and then reads next 10 bytes. Useful if you only want to do something once a message has been recieved. returns whole message
	// justReadOneByte: just reads the next byte, and deals with it. Useful if you need to do other stuff with your program and want to just deal with the messages when they're ready. returns a bool, when finished message can be got with getMessage. if(readonebyte) then getMessage.
	// parse message: does the same thing for both, either one at a time or all togther.
	// maybe instead of parsing one at a time we can wait for ~12 bytes in the buffer and then read them one at a time from there to find sync. then just dump the rest into the array? need to deal with pid and dlc... might be the same amount of waiting really.  

  if (Serial1.available()) {
    //timestampOfLastByte = millis();
    prevByte = incomingByte;
    incomingByte = Serial1.read();

    // This works by waiting for a Frame Error, and then using that to sync up the rest of the bytes.
    // After the SYNC byte we increment the LINcounter and cascade down this if/else nest as each new byte comes in.
    
    if (prevByte == 0 && bit_is_set(UCSR1A, FE1) != 0) { // 0: if the previous byte was all 0s and there is a Frame Error You're at the Sync byte!
      LINcounter = SYNC; // 0
    }
    else if (LINcounter == PID) { // 1: if current byte is PID, calculate ID and DLC  
      LINworkingData.id = incomingByte & 0x3F;
      LINworkingData.dlc = calculateDLC(LINworkingData.id); // The length of the data packet is based off ID value.
    }
    else if (LINcounter == DATA + LINworkingData.dlc) // 2-9: if the current byte is the checksum, skip the rest of the data array and write to checksum location.
      // Loop will cycle through once for each data byte.
      // If the data length is 2, it will loop twice and then jump to the checksum.
      LINcounter += 8 - LINworkingData.dlc;

    if (LINcounter < 11) { // We're looking for 12 bytes once we've found the sync byte
      LINworkingData.rawData[LINcounter] = incomingByte; // Save the byte and continue.
      LINcounter++;
    }

    if (LINcounter > CHECKSUM) { // 11: if we've reached the checksum, and checksum is valid...
			if (legacyMode || LINworkingData.id >= 0x3C) { // 60
				if (validateDataChecksumClassic(LINworkingData)) {
				  LINverifiedData = LINworkingData;
					return true;
				}
			} else {
				if (validateDataChecksumEnhanced(LINworkingData)) {
				  LINverifiedData = LINworkingData;	
					return true;
				}
			}
		}
		return false;
  }
}

LINtransceiver::validateIDChecksum(uint8_t pid) {
	uint8_t pidSum;
	// Is this Exclusive OR, or some sort of modulo/addition?
	// The parity bits are calculated as follows:
  //  P0 = ID0 ⊕ ID1 ⊕ ID2 ⊕ ID4
  //  P1 = !(ID1 ⊕ ID3 ⊕ ID4 ⊕ ID5)

}

LINtransceiver::validateDataChecksumClassic(LIN_Data_t LIN_Data) {
	// Classic Checksum is imverted eight bit sum of all data bytes in the message. 
	// "Always used for diagnostic frames."
	// "Is used for identifiers 0x3C to 0x3F (60 to 63)"
	
  uint8_t byteSum;
	
  for (uint8_t i = 0; i < 8; i++) { // 8 data bytes
    byteSum += LIN_Data.data[i];
	}
	
	// Sum of data + checksum should equal 0xFF
  return (byteSum == 0xFF); 
}

LINtransceiver::validateDataChecksumEnhanced(LIN_Data_t LIN_Data) {
	// The enhanced checksum is inverted eight bit sum of all data bytes in the message and protected identifier. 
	// "Is used for identifiers 0x00 to 0x3B (0 to 59)"
	
  uint8_t byteSum;
	
  for (uint8_t i = 0; i < 9; i++) { // 1 pid and 8 data bytes
    byteSum += LIN_Data.RawData[i+1];
	}
	
	// Sum of data + checksum should equal 0xFF
  return (byteSum == 0xFF); 
}

LINtransceiver::calculateDLC(uint8_t id) { // calculate the number of data bytes based on ID value
	
  if (id <= 0x1F) // 31
    return 2;
  else if (id <= 0x2F) // 47
    return 4;
  else if (id <= 0x3F) // 63
    return 8;
	else 
		return 0; // this should never happen. 6 bit id max is 63
}


LINtransceiver::getData() {
	return LINverifiedData;
}

LINtransceiver::getTimestampOfLastByte() {
	return timestampOfLastByte;
}
