#include "Arduino.h"
#include "TM1638liteEx.h"


#define CLOCK_DELAY_US    1

#define TM_BRIGHT_ADR     0x88
#define TM_BRIGHT_MASK    0x07



TM1638liteEx::TM1638liteEx(uint8_t strobe, uint8_t clock, uint8_t data, bool highFreq, bool clockInverted) {
  this->strobe = strobe;
  this->data = data;
  this->clock = clock;
  this->highFreq = highFreq;
  this->clockInverted = clockInverted;
  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  sendCommand(ACTIVATE);
  reset();
}

void TM1638liteEx::shiftOutEx(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
  if (highFreq) {
	  for (uint8_t i = 0; i < 8; i++)  {
		  if (bitOrder == LSBFIRST)
  			digitalWrite(dataPin, !!(val & (1 << i)));
		  else	
  			digitalWrite(dataPin, !!(val & (1 << (7 - i))));
			
		  digitalWrite(clockPin, (clockInverted ? HIGH : LOW));
      delayMicroseconds(CLOCK_DELAY_US);
		  digitalWrite(clockPin, (clockInverted ? LOW : HIGH));		
	  }
  } else {
    shiftOut(dataPin, clockPin, bitOrder, val);
  }
}

uint8_t TM1638liteEx::shiftInEx(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
  if (highFreq) {
	  uint8_t value = 0;  
	  for (uint8_t i = 0; i < 8; ++i) {
  		digitalWrite(clockPin, (clockInverted ? HIGH : LOW));
      delayMicroseconds(CLOCK_DELAY_US);
		  if (bitOrder == LSBFIRST)
  			value |= digitalRead(dataPin) << i;
		  else
  			value |= digitalRead(dataPin) << (7 - i);
		  digitalWrite(clockPin, (clockInverted ? LOW : HIGH));
	  }
	  return value;
  } else {
    return shiftIn(dataPin, clockPin, bitOrder);
  }
}

void TM1638liteEx::sendCommand(uint8_t value)
{
  digitalWrite(strobe, LOW);
  shiftOutEx(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void TM1638liteEx::reset() {
  sendCommand(WRITE_INC); // set auto increment mode
  digitalWrite(strobe, LOW);
  shiftOutEx(data, clock, LSBFIRST, 0xc0);   // set starting address to 0
  for (uint8_t i = 0; i < 16; i++)
  {
    shiftOutEx(data, clock, LSBFIRST, 0x00);
  }
  digitalWrite(strobe, HIGH);
}

void TM1638liteEx::brightness(uint8_t brightness) {
    // brightness 0 to 7
    uint8_t value = TM_BRIGHT_ADR + (TM_BRIGHT_MASK & brightness);
    sendCommand(value);
}

uint8_t TM1638liteEx::readButtons() {
  uint8_t buttons = 0;
  digitalWrite(strobe, LOW);
  shiftOutEx(data, clock, LSBFIRST, BUTTONS);
  pinMode(data, INPUT);
  
  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t v = shiftInEx(data, clock, LSBFIRST) << i;
    buttons |= v;
  }
  
  pinMode(data, OUTPUT);
  digitalWrite(strobe, HIGH);
  return buttons;
}

void TM1638liteEx::setLED(uint8_t position, uint8_t value)
{
  pinMode(data, OUTPUT);
  sendCommand(WRITE_LOC);
  digitalWrite(strobe, LOW);
  shiftOutEx(data, clock, LSBFIRST, 0xC1 + (position << 1));
  shiftOutEx(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void TM1638liteEx::setLEDs(uint8_t value) {
  //Serial.println(value);
  for (uint8_t position = 0; position < 8; position++) {
    setLED(position, value & 1);
    value = value >> 1;
  }
}

void TM1638liteEx::displayText(String text) {
  uint8_t length = text.length();

  for (uint8_t i = 0; i < length; i++) {
    for (uint8_t position = 0; position < 8; position++) {
      displayASCII(position, text[position]);
    }
  }
}

void TM1638liteEx::displaySS(uint8_t position, uint8_t value) { // call 7-segment
  sendCommand(WRITE_LOC);
  digitalWrite(strobe, LOW);
  shiftOutEx(data, clock, LSBFIRST, 0xC0 + (position << 1));
  shiftOutEx(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void TM1638liteEx::displayRAW(uint8_t position, uint16_t value) { // call 7-segment
  sendCommand(WRITE_LOC);
  digitalWrite(strobe, LOW);
  shiftOutEx(data, clock, LSBFIRST, 0xC0 + (position << 1));
  shiftOutEx(data, clock, MSBFIRST, highByte(value));
  digitalWrite(strobe, HIGH);
}

void TM1638liteEx::displayASCII(uint8_t position, uint8_t ascii) {
  displaySS(position, ss[ascii]);
}

void TM1638liteEx::displayHex(uint8_t position, uint8_t hex) {
  displaySS(position, hexss[hex]);
}
