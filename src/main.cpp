#include <Arduino.h>

#include "Button2.h"
#include "elapsedMillis.h"

/* --------------------------------------------------------------- */

//#define BUTTON_PIN        PA1
//#define PWM_OUT           PA3

#define BUTTON_PIN        PA0
#define PWM_OUT           PA1

#define DEBOUNCE_TIME     5     // ms
#define LONG_PRESS_TIME   500   // ms

#define INITIAL_PWM_SET   15    // %

#define LED_TMO           500   // ms
#define PWM_TMO           10    // ms
#define BUTTON_PRESED_TMO 10    // ms

/* --------------------------------------------------------------- */

typedef enum {
  pwmRise,
  pwmFall,
} tePWMDirection;

Button2 button = Button2(BUTTON_PIN);

static int8_t ledSetPoint = 0;
static int8_t ledSetPointOld = 0;

// Timers
elapsedMillis ledTMO;
elapsedMillis pwmTMO;
elapsedMillis buttonPressedTMO;

// Helpers
static bool isTurnedOn() {
  return ledSetPoint != 0;
}

// BUTTON CLICK HANDLERS
void clickHandler(Button2& btn) {
  // at first press set to 50%
  if (ledSetPoint == ledSetPointOld) ledSetPointOld = INITIAL_PWM_SET;

  if (isTurnedOn()) {               // is turned ON?
    ledSetPointOld = ledSetPoint;   // store old value
    ledSetPoint = 0;                // set target percentage
  } else {
    ledSetPoint = ledSetPointOld;   // restore target percentage
    ledSetPointOld = 0;             // clear old value
  }
}

static bool longPressInProcess = false;

// Long press detected
void longClickHandler(Button2& btn) {
  longPressInProcess = true;
  buttonPressedTMO = 0;
}

// Long press ended (button released)
void longClickReleasedHandler(Button2& btn) {
  longPressInProcess = false;
}

void doubleClickHandler(Button2& btn) {
  if (!isTurnedOn()) return;

  if (ledSetPoint == 2) {
    ledSetPoint = 5;
  } else {
    ledSetPoint = 2;
  }
}

void tripleClickHandler(Button2& btn) {
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

// Long press and hold button processor
static void buttonStillPressed()
{
  static tePWMDirection pwmDirection = pwmRise;

  if (isTurnedOn()                                // is turned ON?
      && buttonPressedTMO > BUTTON_PRESED_TMO) {  // AND button was holded and TMO is passed (slowing down changing)
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

    buttonPressedTMO = 0;
  }
}

// HW PWM scale is 0-255
static uint32_t percentToDuty(uint8_t percents) {
  return uint32_t(255.0 / 100.0 * percents);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  button.setDebounceTime(DEBOUNCE_TIME);
  button.setClickHandler(clickHandler);

  button.setLongClickTime(LONG_PRESS_TIME);
  button.setLongClickDetectedHandler(longClickHandler);
//  button.setLongClickDetectedRetriggerable(true);
  button.setLongClickHandler(longClickReleasedHandler);

  button.setDoubleClickHandler(doubleClickHandler);

  button.setTripleClickHandler(tripleClickHandler);

  pinMode(PWM_OUT, OUTPUT);
}

// PWM Output processing
static void pwmProcess()
{
  static uint8_t pwmVal = 0;
  bool bypassPWMtmo = longPressInProcess;

  if (pwmTMO >= PWM_TMO || bypassPWMtmo) {  // slowing down processing or if button is long pressed and holded -> care takes buttonStillPressed()
    if (ledSetPoint != pwmVal)              // process only if something changed
    {
      tePWMDirection pwmDirection = (ledSetPoint > pwmVal) ? pwmRise : pwmFall;

      switch (pwmDirection) 
      {
        case pwmRise:
          pwmVal++;
        break;

        case pwmFall:
          pwmVal--;
        break;
      }

      analogWrite(PWM_OUT, percentToDuty(pwmVal));
    }

    pwmTMO = 0;
  }
}

// Status LED Blinker
static void ledProcess()
{
  static bool ledStatus = false;

  if (ledTMO >= LED_TMO) {
    ledStatus = !ledStatus;
    digitalWrite(LED_BUILTIN, ledStatus);
    ledTMO = 0;
  }
}

void loop() {
  ledProcess();
  
  pwmProcess();
  
  button.loop();

  if (longPressInProcess) {
    buttonStillPressed();
  }

  // workaround: recover button state (depress not always registereg by Button2 library longClickReleasedHandler())
  if (!button.isPressed()) {
    longPressInProcess = false;
  } 
}
