#pragma once

#ifndef CmdProcessor_h
#define CmdProcessor_h

#include "StateM.h"
#include "Arduino.h"

class CmdProcDataSet {
    public:
        uint8_t ledSP;
        StateM::State cState;
        CmdProcDataSet(void) { };
        CmdProcDataSet(uint8_t _ledSP, StateM::State _cState) { 
            ledSP = _ledSP;
            cState = _cState;
        }
};

class CmdProcessor {
    public:
        CmdProcessor(uint8_t _buffSize);
        CmdProcDataSet getDatSet();
        bool cmdProcess(String cmdStr, CmdProcDataSet _dataSet);
};

#endif
