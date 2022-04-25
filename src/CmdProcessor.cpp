#include "CmdProcessor.h"

CmdProcDataSet dataSet;
uint8_t buffSize;

typedef struct {
  String  cmd;
  String  subCmd;
  bool    argNeeded;
  bool (* f_ptr)(String s);
  //uint8_t (* f_ptr)(uint8_t *, uint8_t *);
} tsCmds;

static bool processHelp(String s);
static bool processGetPWM(String s);
static bool processSetPWM(String s);

tsCmds cmds[] = {
  "?",    "",     false,  processHelp,
  "get",  "pwm",  false,  processGetPWM,
  "set",  "pwm",  true,   processSetPWM,
};

// ------------------------------------------------------------

CmdProcessor::CmdProcessor(uint8_t _buffSize) {
    buffSize = _buffSize;
}

// ------------------------------------------------------------

CmdProcDataSet CmdProcessor::getDatSet() {
    return dataSet;
}

// ------------------------------------------------------------

static bool processHelp(String s)
{
  Serial.println("Help: <get|set> <pwm> [value]");
  dataSet.cState = StateM::S_NEEDINPUT;
  return true;
}

static bool processGetPWM(String s)
{
    Serial.printf("Current SetPoint = %d\r\n", dataSet.ledSP);
    dataSet.cState = StateM::S_NEEDINPUT;
    return true;
}

static bool processSetPWM(String s)
{
  bool res = false;

  char tempBuff[buffSize] = { 0 };
  memcpy(tempBuff, s.c_str(), s.length());

  // check for all digits in userinput
  uint8_t idx = 0;
  do {
      if (0 == isdigit(tempBuff[idx])) 
      {
        Serial.printf("ERROR: pos %d <%s>\r\n", idx, s.c_str());
        dataSet.cState = StateM::S_ERROR;
        break;
      }
      idx++;
  } while (idx < s.length());

  // further check for valid number and set new ledSetpoint
  if (dataSet.cState != StateM::S_ERROR) {
    long i = s.toInt();
    if (i >= 0 && i <= 100) {
      Serial.printf("Current SetPoint = %d\r\n", dataSet.ledSP);
      Serial.printf("New SetPoint = %d\r\n", i);
      dataSet.ledSP = i;
      dataSet.cState = StateM::S_PWM_IN_PROGRESS;
      res = true;
    } else {
      Serial.printf("ERROR: Value <%d> [0-100]\r\n", i);
      dataSet.cState = StateM::S_ERROR;
    }
  }

  return res;
}

// ------------------------------------------------------------

#define COMMON_NUMEL(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

static String findNextCmd(String s, String s1, bool argNeeded)
{
  String s_out = "";
  if (s.length() >= s1.length()) {
    if (s.startsWith(s1)) {

      if (argNeeded == true) {
        int idx = s.indexOf(" ");
        if (idx != -1) {
          idx += 1;
          s_out = s.substring(idx);
        }
      }

      // Serial.printf(">>%s\r\n", tempCmdStr.c_str());

      s_out = s.substring(s.indexOf(" ") + 1);
    }
  }

  return s_out;
}

bool CmdProcessor::cmdProcess(String cmdStr, CmdProcDataSet _dataSet)
{
  String tempCmdStr;

  bool res = false;

  dataSet = _dataSet;

  for (uint8_t i = 0; i < COMMON_NUMEL(cmds); i++) {
    tempCmdStr = findNextCmd(cmdStr, cmds[i].cmd, false);
    if (tempCmdStr.length() > 0) {
      // Serial.printf(">%s\r\n", tempCmdStr.c_str());
      tempCmdStr = findNextCmd(tempCmdStr, cmds[i].subCmd, cmds[i].argNeeded);
      if (tempCmdStr.length() > 0) {
        if (cmds[i].argNeeded == false
            || (cmds[i].argNeeded == true && tempCmdStr.length() > 0)) {
          res = (cmds[i].f_ptr(tempCmdStr));
          break;
        }
      }
    }
  }

  return res;
}
