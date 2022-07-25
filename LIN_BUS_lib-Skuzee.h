// LIN_BUS_lib-Skuzee
// LIN_Bus transceiver library, currently only receiving is written.
// Has functions for data checksum and advanced checksum for LIN 1.3 and LIN 2.x support
// Uses UCSR1A register to detect Frame Error, might not work on all arduino.

#pragma once

// Includes
#include <Arduino.h>

// Define which parts of the data frame do what. Sync, PID, 2-8 Data bytes, Checksum, Breakfield.
#define SYNC 0
#define PID 1
#define DATA 2
#define CHECKSUM 10
#define BREAKFIELD 11

// raw LIN data.
typedef union {
  uint8_t rawData[11]; // Easily read/write first 11 bytes.
  struct {
	// Order is important, in order as received.
    uint8_t sync; // Should always equal 0x55 (01010101)
    uint8_t pid; // protected identifier; id value and 2 parity bits
    uint8_t data[8]; 
    uint8_t dataChecksum; //data checksum
		
		bool checksumValid;
    uint8_t dlc; //data length in bytes
    uint8_t id; // id value no parity bits, used to calculate dlc.
  };
} LIN_Data_t;


class LINtransceiver {
	public:
		LINtransceiver(uint8_t _TX_Pin, uint8_t _RX_Pin, uint16_t _Baud, uint8_t _CS_Pin);
		
		LINtransceiver(uint8_t _TX_Pin, uint8_t _RX_Pin, uint16_t _Baud, uint8_t _CS_Pin, bool _legacyMode);
		
		void initLIN(void); // start serial connections, configure pins
		bool read(void); // returns true when LIN Data is ready.
		bool validateIDChecksum(uint8_t pid);
		bool validateDataChecksumClassic(LIN_Data_t LIN_Data);
		bool validateDataChecksumEnhanced(LIN_Data_t LIN_Data);
		uint8_t calculateDLC(uint8_t id); // returns Data Length Control (Number of bytes in the data payload)
		LIN_Data_t getData(void);
		unsigned long getTimestampOfLastByte(void);

	private:
		uint8_t LIN_TX_Pin = 1;
		uint8_t LIN_RX_Pin = 0;
		unsigned long LIN_Baud = 9600UL;
		uint8_t CS_Pin = 2;
		bool legacyMode = false;
		
		LIN_Data_t LINworkingData; // incoming data written to working file, then copied to LINverifiedData if Checksum is okay.
		LIN_Data_t LINverifiedData;
		uint8_t incomingByte;
		uint8_t prevByte;
		int8_t LINcounter; // data_counter counts LINworkingData.rawData position, and corresponds to data type.
		unsigned long timestampOfLastByte;
};
