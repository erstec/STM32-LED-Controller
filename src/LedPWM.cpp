#include "LedPWM.h"

uint8_t     initialPWM;
uint8_t     buttonTMO;
uint32_t    outPIN;

// ----------------------------------------------------------------------

LedPWM::LedPWM(uint8_t _initialPWM, uint8_t _buttonTMO, uint32_t _outPIN) 
{
    initialPWM  = _initialPWM;
    buttonTMO   = _buttonTMO;
    outPIN      = _outPIN;

    pinMode(outPIN, OUTPUT);
}

// ----------------------------------------------------------------------

int LedPWM::getLEDsetPoint() {
    return ledSetPoint;
}
void LedPWM::setLEDsetPoint(uint8_t val) {
    ledSetPoint = val;
}

int LedPWM::getLEDoldSetPoint() {
    return ledSetPointOld;
}

void LedPWM::setLEDoldSetPoint(uint8_t val) {
    ledSetPointOld = val;
}

// ----------------------------------------------------------------------

bool LedPWM::isTurnedOn() {
  return ledSetPoint != 0;
}

// ----------------------------------------------------------------------

void LedPWM::clickProcess()
{
    // at first press set to 50%
    if (ledSetPointOld == ledSetPoint) ledSetPointOld = initialPWM;

    if (isTurnedOn()) {                 // is turned ON?
        ledSetPointOld = ledSetPoint;   // store old value
        ledSetPoint = 0;                // set target percentage
    } else {
        ledSetPoint = ledSetPointOld;   // restore target percentage
        ledSetPointOld = 0;             // clear old value
    }
}

void LedPWM::doubleClickProcess()
{
    if (!isTurnedOn()) return;

    if (ledSetPoint == 2) {
        ledSetPoint = 5;
    } else {
        ledSetPoint = 2;
    }
}

void LedPWM::trippleClickProcess()
{
    if (!isTurnedOn()) return;
  
    // from 2/5% -> 10%
    // 10-15-20-25-50-75-100
    if (ledSetPoint == 2 || ledSetPoint == 5) {
        ledSetPoint = 10;
    }
    else if (ledSetPoint > 0 && ledSetPoint <= 10)
    {
        ledSetPoint = 15;
    }
    else if (ledSetPoint > 10 && ledSetPoint <= 15) 
    {
        ledSetPoint = 20;
    }
    else if (ledSetPoint > 15 && ledSetPoint <= 20) 
    {
        ledSetPoint = 25;
    }
    else if (ledSetPoint > 20 && ledSetPoint <= 25) 
    {
        ledSetPoint = 50;
    }
    else if (ledSetPoint > 25 && ledSetPoint <= 50) 
    {
        ledSetPoint = 75;
    }
    else if (ledSetPoint > 50 && ledSetPoint <= 75) 
    {
        ledSetPoint = 100;
    }
    else 
    {
        ledSetPoint = 10;
    }
}

void LedPWM::longPressProcess(uint32_t tmo)
{
  static tePWMDirection pwmDirection = pwmRise;

  if (isTurnedOn()                      // is turned ON?
      && tmo > buttonTMO) {             // AND button was holded and TMO is passed (slowing down changing)
    switch (pwmDirection) {
      case pwmRise:
        ledSetPoint++;
        if (ledSetPoint > 100) {
          ledSetPoint = 100;
          pwmDirection = pwmFall;       // reverse direction
        }
      break;

      case pwmFall:
        ledSetPoint--;
        if (ledSetPoint < 1) {          // don't turn off completely
          ledSetPoint = 1;
          pwmDirection = pwmRise;       // reverse direction
        }
      break;
    }
  }
}

// ----------------------------------------------------------------------

// HW PWM scale is 0-255
uint32_t percentToDuty(uint8_t percents) {
  return uint32_t(255.0 / 100.0 * percents);
}

// PWM Output processing
bool LedPWM::processPWM()
{
    static uint8_t pwmVal = 0;

    if (ledSetPoint != pwmVal)              // process only if something changed
    {
        tePWMDirection pwmDirection = (ledSetPoint > pwmVal) ? pwmRise : pwmFall;

        switch (pwmDirection) 
        {
            case pwmRise:
                pwmVal++;
                Serial.printf("PWM %d->%d\r\n", pwmVal, ledSetPoint);
            break;

            case pwmFall:
                pwmVal--;
                Serial.printf("PWM %d<-%d\r\n", ledSetPoint, pwmVal);
            break;
        }
        
        analogWrite(outPIN, percentToDuty(pwmVal));

        return true;
    } 
    else 
    {
        return false;
    }
}
