#pragma once

#ifndef StateM_h
#define StateM_h

class StateM
{
    public:
        typedef enum {
            S_INIT,
            S_PWM_IN_PROGRESS,
            S_NEEDINPUT,
            S_WAITING,
            S_TIMEOUT,
            S_PROCESSING,
            S_ERROR
        } State;
        
        StateM(void) { state = S_INIT; };
        operator State () const { return state; }
        StateM & operator = (State val) { state = val; return *this; }
        
    private:    
        State state;
};

#endif
