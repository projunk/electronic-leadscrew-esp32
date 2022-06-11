// Clough42 Electronic Leadscrew
// https://github.com/clough42/electronic-leadscrew
//
// MIT License
//
// Copyright (c) 2019 James Clough
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include <compat.h>
#include <Configuration.h>
#include <ControlPanel.h>

// Number of times a key state must be read consecutively to be considered stable
#define MIN_CONSECUTIVE_READS 3


ControlPanel :: ControlPanel()
{
    tm = new TM1638liteEx(ESP32_TM1638_STROBE_PIN, ESP32_TM1638_CLOCK_PIN, ESP32_TM1638_DATA_PIN, true, true);
    this->rpm = 0;
    this->value = NULL;
    this->leds.all = 0;
    this->keys.all = 0;
    this->stableKeys.all = 0;
    this->stableCount = 0;
    this->message = NULL;
    this->brightness = 3;
}

Uint16 ControlPanel :: lcd_char(Uint16 x)
{
    static const Uint16 table[] = {
        0b1111110000000000, // 0
        0b0110000000000000, // 1
        0b1101101000000000, // 2
        0b1111001000000000, // 3
        0b0110011000000000, // 4
        0b1011011000000000, // 5
        0b1011111000000000, // 6
        0b1110000000000000, // 7
        0b1111111000000000, // 8
        0b1111011000000000, // 9
        0b0000000100000000  // .
    };
    if( x < sizeof(table) ) {
        return table[x];
    }
    return table[sizeof(table)-1];
}

void ControlPanel :: sendData()
{
    tm->brightness(this->brightness);

    for(int i=0; i < 8; i++) {
        if( this->message != NULL )
        {
            tm->displayRAW(i,this->message[i]);
        }
        else
        {
            tm->displayRAW(i,this->sevenSegmentData[i]);
        }
    }
    tm->setLEDs(this->leds.all);
}

void ControlPanel :: decomposeRPM()
{
    Uint16 rpm = this->rpm;
    int i;

    for(i=3; i>=0; i--) {
        this->sevenSegmentData[i] = (rpm == 0 && i != 3) ? 0 : lcd_char(rpm % 10);
        rpm = rpm / 10;
    }
}

void ControlPanel :: decomposeValue()
{
    if( this->value != NULL )
    {
        int i;
        for( i=0; i < 4; i++ )
        {
            this->sevenSegmentData[i+4] = this->value[i];
        }
    }
}

KEY_REG ControlPanel :: readKeys(void)
{
    KEY_REG keys;
    keys.all = tm->readButtons();
    return keys;
}

KEY_REG ControlPanel :: getKeys()
{
    KEY_REG newKeys;
    static KEY_REG noKeys;

    newKeys = readKeys();
    if( isValidKeyState(newKeys) && isStable(newKeys) && newKeys.all != this->keys.all ) {
        KEY_REG previousKeys = this->keys; // remember the previous stable value
        this->keys = newKeys;

        if( previousKeys.all == 0 ) {     // only act if the previous stable value was no keys pressed
            return newKeys;
        }
    }
    return noKeys;
}

bool ControlPanel :: isValidKeyState(KEY_REG testKeys) {
    // filter out any states with multiple keys pressed (bad communication filter)
    switch(testKeys.all) {
    case 0:
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
    case 128:
        return true;
    }

    return false;
}

bool ControlPanel :: isStable(KEY_REG testKeys) {
    // don't trust any read key state until we've seen it multiple times consecutively (noise filter)
    if( testKeys.all != stableKeys.all )
    {
        this->stableKeys = testKeys;
        this->stableCount = 1;
    }
    else
    {
        if( this->stableCount < MIN_CONSECUTIVE_READS )
        {
            this->stableCount++;
        }
    }

    return this->stableCount >= MIN_CONSECUTIVE_READS;
}

void ControlPanel :: setMessage( const Uint16 *message )
{
    this->message = message;
}

void ControlPanel :: setBrightness( Uint16 brightness )
{
    if( brightness > 8 ) brightness = 8;

    this->brightness = brightness;
}

void ControlPanel :: refresh()
{
    decomposeRPM();
    decomposeValue();

    sendData();
}






