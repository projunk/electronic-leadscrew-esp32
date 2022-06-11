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


#ifndef __CORE_H
#define __CORE_H

#include <compat.h>
#include "StepperDrive.h"
#include "Encoder.h"
#include "ControlPanel.h"
#include "Tables.h"
#include <esp_attr.h>


class Core
{
private:
    Encoder *encoder;
    StepperDrive *stepperDrive;

#ifdef USE_FLOATING_POINT
    float feed;
    float previousFeed;
#else
    const FEED_THREAD *feed;
    const FEED_THREAD *previousFeed;
#endif // USE_FLOATING_POINT

    int16 feedDirection;
    int16 previousFeedDirection;

    Uint32 previousSpindlePosition;

    int64 feedRatio(Uint64 count);

    bool powerOn;

public:
    Core( Encoder *encoder, StepperDrive *stepperDrive );

    void setFeed(const FEED_THREAD *feed);
    void setReverse(bool reverse);
    Uint16 getRPM(void);
    bool isAlarm();

    bool isPowerOn();
    void setPowerOn(bool);

    void IRAM_ATTR ISR( void );
};

inline void Core :: setFeed(const FEED_THREAD *feed)
{
#ifdef USE_FLOATING_POINT
    this->feed = (float)feed->numerator / feed->denominator;
#else
    this->feed = feed;
#endif // USE_FLOATING_POINT
}

inline Uint16 Core :: getRPM(void)
{
    return encoder->getRPM();
}

inline bool Core :: isAlarm()
{
    return this->stepperDrive->isAlarm();
}

inline bool Core :: isPowerOn()
{
    return this->powerOn;
}

inline int64 Core :: feedRatio(Uint64 count)
{
#ifdef USE_FLOATING_POINT
    return ((float)count) * this->feed * feedDirection;
#else // USE_FLOATING_POINT
    return ((long long)count) * feed->numerator / feed->denominator * feedDirection;
#endif // USE_FLOATING_POINT
}

inline void IRAM_ATTR Core :: ISR( void )
{
#ifdef USE_FLOATING_POINT
    if( !IS_FLOAT_NULL(this->feed) ) {
#else
    if( this->feed != NULL ) {
#endif        
        // read the encoder
        Uint64 spindlePosition = encoder->getPosition();

        // calculate the desired stepper position
        int64 desiredSteps = feedRatio(spindlePosition);
        stepperDrive->setDesiredPosition(desiredSteps);

        // if the feed or direction changed, reset sync to avoid a big step
        if( feed != previousFeed || feedDirection != previousFeedDirection) {
            stepperDrive->setCurrentPosition(desiredSteps);
        }

        // remember values for next time
        previousSpindlePosition = spindlePosition;
        previousFeedDirection = feedDirection;
        previousFeed = feed;

        // service the stepper drive state machine
        stepperDrive->ISR();        
    }
}


#endif // __CORE_H
