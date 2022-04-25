#pragma once

#ifndef LedPWM_h
#define LedPWM_h

#include "Arduino.h"

class LedPWM
{
    private:
        typedef enum {
            pwmRise,
            pwmFall,
        } tePWMDirection;

        uint8_t ledSetPoint = 0;
        uint8_t ledSetPointOld = 0;

    public:
        LedPWM(uint8_t _initialPWM, uint8_t _buttonTMO, uint32_t _outPIN);
        int getLEDsetPoint();
        void setLEDsetPoint(uint8_t val);
        int getLEDoldSetPoint();
        void setLEDoldSetPoint(uint8_t val);
        bool isTurnedOn();
        void clickProcess();
        void doubleClickProcess();
        void trippleClickProcess();
        void longPressProcess(uint32_t tmo);
        bool processPWM();
};

#endif
