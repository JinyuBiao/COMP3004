#include "patient.h"

Patient::Patient(bool a, StateType d):
    isAdult(a),currState(d)
{}

Patient::~Patient()
{}

void Patient::changeAge()
{
    isAdult = !isAdult;
}

void Patient::setState(StateType s)
{
    currState = s;
}

bool Patient::notChild()
{
    return isAdult;
}

StateType Patient::getState()
{
    return currState;
}

QString Patient::getStateString()
{
    switch(currState){
        case fibrillation:
            return("ventricular fibrillation");
        case tachycardia:
            return("ventricular tachycardia");
        case other:
            return("asystole");
        case dead:
            return("dead");
        case healthy:
            return("healthy");
        default:
            return("unknown");
    }
}
