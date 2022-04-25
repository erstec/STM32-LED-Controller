#include <Arduino.h>

#include "Button2.h"
#include "elapsedMillis.h"
#include "StateMachine.h"

#include "StateM.h"
#include "LedPWM.h"
#include "CmdProcessor.h"

/* --------------------------------------------------------------- */

#define BUTTON_PIN        PA1
#define PWM_OUT           PA3

#define DEBOUNCE_TIME     5     // ms
#define LONG_PRESS_TIME   500   // ms

#define INITIAL_PWM_SET     15  // %

#define LED_TMO             500 // ms
#define PWM_TMO             10  // ms
#define BUTTON_PRESSED_TMO  10  // ms

#define USERWAITTIME 10000 // milliseconds
#define BUFF_SIZE 32

/* --------------------------------------------------------------- */

StateM state_m = StateM();
LedPWM ledPWM = LedPWM(INITIAL_PWM_SET, BUTTON_PRESSED_TMO, PWM_OUT);
CmdProcessor cmdProc = CmdProcessor(BUFF_SIZE);

elapsedMillis timeout;
char ringBuffer[BUFF_SIZE];
uint8_t ringBuffIdx = 0;

/* --------------------------------------------------------------- */

Button2 button = Button2(BUTTON_PIN);

StateMachine userInputMachine = StateMachine();
State* S_INIT = userInputMachine.addState([](){
  Serial.println("S_INIT");
});
State* S_PWM_IN_PROGRESS = userInputMachine.addState([](){
  Serial.println("S_PWM_IN_PROGRESS");
});
State* S_NEEDINPUT = userInputMachine.addState([](){
  Serial.println("S_NEEDINPUT");
});
State* S_WAITING = userInputMachine.addState([](){
  Serial.println("S_WAITING");
});
State* S_TIMEOUT = userInputMachine.addState([](){
  Serial.println("S_TIMEOUT");
});
State* S_PROCESSING = userInputMachine.addState([](){
  Serial.println("S_PROCESSING");
});
State* S_FINISHED = userInputMachine.addState([](){
  Serial.println("S_FINISHED");
});
State* S_ERROR = userInputMachine.addState([](){
  Serial.println("S_ERROR");
});

// Timers
elapsedMillis ledTMO;
elapsedMillis pwmTMO;
elapsedMillis buttonPressedTMO;

// BUTTON CLICK HANDLERS
void clickHandler(Button2& btn) {
  ledPWM.clickProcess();
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
  ledPWM.doubleClickProcess();
}

void tripleClickHandler(Button2& btn) {
  ledPWM.trippleClickProcess();
}

// Long press and hold button processor
static void buttonStillPressed()
{
  ledPWM.longPressProcess(buttonPressedTMO);
  buttonPressedTMO = 0;
}

// PWM Output processing
static void pwmProcess()
{
  bool bypassPWMtmo = longPressInProcess;

  if (pwmTMO >= PWM_TMO || bypassPWMtmo) {  // slowing down processing or if button is long pressed and holded -> care takes buttonStillPressed()
    if (ledPWM.processPWM()) {
      state_m = StateM::S_PWM_IN_PROGRESS;
    } else {
      if (state_m == StateM::S_PWM_IN_PROGRESS) {
        state_m = StateM::S_NEEDINPUT;
      }
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

    userInputMachine.run();
  }
}

static void userInput()
{
  switch (state_m)
  {
    case StateM::S_INIT:
        delay(1000);
        Serial.println("Listening for user commands...");
        state_m = StateM::S_NEEDINPUT;
        break;
    case StateM::S_NEEDINPUT:
        Serial.print("Enter PWM percentage [0-100]: ");
        memset(ringBuffer, 0x00, sizeof(ringBuffer));
        ringBuffIdx = 0;
        timeout = 0;
        state_m = StateM::S_WAITING;
        break;
    case StateM::S_WAITING:
        if (Serial.available())
        {
            //char userinput = toupper(Serial.read());
            char userinput = tolower(Serial.read());

            Serial.printf("%c", userinput); // echo character back to user

            ringBuffer[ringBuffIdx++] = userinput;
            if (ringBuffIdx >= sizeof(ringBuffer)) {
              Serial.println("RX buffer overflow");
              state_m = StateM::S_ERROR;
              break;
            }

            // CRLF received
            if (ringBuffer[ringBuffIdx - 2] == '\r' && ringBuffer[ringBuffIdx - 1] == '\n')
            {
                state_m = StateM::S_PROCESSING;
            }
            break;
        }
        // receiving (entering) in progress
        else if (ringBuffIdx != 0 && timeout > USERWAITTIME)
        {
            state_m = StateM::S_TIMEOUT;
        }
        // no one char entered yet
        else if (ringBuffIdx == 0) {
            timeout = 0;
        }
        break;
    case StateM::S_TIMEOUT:
        Serial.println("<timeout>");
        state_m = StateM::S_NEEDINPUT;
        break;
    case StateM::S_PROCESSING:
    {
        String s = ringBuffer;
        s.replace("\r\n", "");  // remove trailing CRLF
        
        CmdProcDataSet dataSet = CmdProcDataSet(ledPWM.getLEDsetPoint(), state_m);
        bool st = cmdProc.cmdProcess(s, dataSet);
        dataSet = cmdProc.getDatSet();
        ledPWM.setLEDsetPoint(dataSet.ledSP);
        state_m = dataSet.cState;
        Serial.printf("Cmd process -> %s\r\n", (st == true) ? "ok" : "error");
        break;
    }
    case StateM::S_PWM_IN_PROGRESS:
      // wait till PWM stabilizes, will switch to S_NEEDINPUT after settles
      break;
    default:
    case StateM::S_ERROR:
        Serial.println("Error!");
        state_m = StateM::S_NEEDINPUT;
        break;
    }
}

void setup() {
  // https://primalcortex.wordpress.com/2020/10/11/stm32-blue-pill-board-arduino-core-and-usb-serial-output-on-platformio/
  // Serial.begin() can be used
  SerialUSB.begin(115200);
  delay(500);
  SerialUSB.println("STM32 LED Controller - v0.0.3 - 2021");

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



  S_INIT->addTransition([](){
    //if () ...                       // eval fn
    //S_INIT->setTransition(0, 1);    // 0->1
    return true;
  },S_NEEDINPUT);     // transition to

  S_NEEDINPUT->addTransition([](){
    //S_NEEDINPUT->setTransition(0, 1);
    return true;
  },S_WAITING);

  S_WAITING->addTransition([](){
    //S_WAITING->setTransition(0, 1); // S_ERROR
    return true;
  },S_ERROR);
  S_WAITING->addTransition([](){
    //S_WAITING->setTransition(0, 1); // S_PROCESSING
    return true;
  },S_PROCESSING);
  S_WAITING->addTransition([](){
    //S_WAITING->setTransition(0, 1); // S_TIMEOUT
    return true;
  },S_TIMEOUT);

  S_TIMEOUT->addTransition([](){
    //S_TIMEOUT->setTransition(0, 1);
    return true;
  },S_NEEDINPUT);

  S_PROCESSING->addTransition([](){
    //S_PROCESSING->setTransition(0, 1);
    return true;
  },S_;

  // S_PWM_IN_PROGRESS->addTransition([](){
  //   S_PWM_IN_PROGRESS->setTransition(0, 1);
  //   return true;
  // },S_);

  S_ERROR->addTransition([](){
    S_ERROR->setTransition(0, 1);
    return true;
  },S_NEEDINPUT);
}

void loop()
{
  userInput();
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

  //machine.run();
}
