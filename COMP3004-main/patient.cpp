#include "patient.h"

Patient::Patient(bool a, bool p, bool c, StateType d, int r):
    isAdult(a),padPlaced(p),standClear(c),currState(d),heartRate(r)
{}

Patient::~Patient()
{}

void Patient::setChild()
{
    isAdult = !isAdult;
}

void Patient::setPad()
{
    padPlaced = !padPlaced;
}

void Patient::setStandClear()
{
    standClear = !standClear;
}

void Patient::setState(StateType s)
{
    currState = s;
}

void Patient::setHeartData()
{
    switch(currState){
        case fibrillation:
            heartAmp = ((rand()%2000)+1);
            if(heartRate==0)
                heartRate = 1; //assume heart rate for fibrillation is 1
            break;
        case tachycardia:
            if(heartRate==0)
                heartRate = (isAdult) ? rand()%100 + 101 : rand()%100 + 121; //generates number between 101 to 200 for adult heart rate, otherwise between 121 to 220 for child heart rate
            if(heartAmp==0)
                heartAmp = 1000; //append to Amplitude array with amplitude 1000 for 20 times
            break;
        case dead:
            if(heartAmp!=0)
                heartRate = 0;
            if(heartAmp!=0)
                heartAmp = 0;
            break;
        case healthy:
            heartRate = (isAdult) ? rand()%41+60 : rand()%41 + 80; //generates number between 60 to 100 for adult heart rate otherwise between 80 to 120 for child heart rate
            heartAmp = 1000; //append to Amplitude array with amplitude 1000 for 20 times
            break;
        case other:
            if(heartRate==0)
                heartRate = (isAdult) ? rand()%59+1 : rand()%79 + 1; //generates number between 1 to 59 for adult heart rate otherwise between 1 to 79 for child heart rate
            if(heartAmp==0)
                heartAmp = 1000; //append to Amplitude array with amplitude 1000 for 20 times
            break;
    }
}

int Patient::getHeartRate()
{
    return heartRate;
}

bool Patient::notInContact()
{
    return standClear;
}

bool Patient::notChild()
{
    return isAdult;
}

bool Patient::hasPad()
{
    return padPlaced;
}

int Patient::getAmp()
{
    return heartAmp;
}

StateType Patient::getState()
{
    return currState;
}
