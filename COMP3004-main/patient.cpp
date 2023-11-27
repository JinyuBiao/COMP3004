#include "patient.h"

Patient::Patient(bool a, bool p, bool c, StateType d, int r):
    isAdult(a),padPlaced(p),standClear(c),currState(d),heartRate(r)
{}

Patient::~Patient()
{}

void Patient::changeAge()
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
    heartRate = 0;//reset heartRate to 0 for next call of setHeartData
    heartAmp = 0;//reset heartAmp to 0 for next call of setHeartData
}

void Patient::setHeartData()
{
    switch(currState){
        case fibrillation:
            heartAmp = ((rand()%1000)+1);//generate random heart amplitude between 1 to 1000, since it is realistic for fibrillation
            if(heartRate==0)
                heartRate = 1; //assume heart rate for fibrillation is 1 (heart rate for fibrillation is actually impossible to detect)
            break;
        case tachycardia:
            if(heartRate==0)
                heartRate = (isAdult) ? rand()%100 + 101 : rand()%100 + 121; //generates heart rate between 101 to 200 for adult heart rate, otherwise between 121 to 220 for child heart rate, if haven't already
            if(heartAmp==0)
                heartAmp = rand()%201 + 800; //generate heart amplitude between 800 to 1000 if haven't already
            break;
        case dead:
            if(heartAmp!=0)
                heartRate = 0;
            if(heartAmp!=0)
                heartAmp = 0;
            break;
        case healthy:
            heartRate = (isAdult) ? rand()%41+60 : rand()%41 + 80; //generates number between 60 to 100 for adult heart rate otherwise between 80 to 120 for child heart rate
            heartAmp = 1000; //generate heart amplitude 1000 if haven't already
            break;
        case other:
            if(heartRate==0)
                heartRate = (isAdult) ? rand()%30+1 : rand()%50 + 1; //generates low heart rate which is 1 to 59 for adult heart rate otherwise between 1 to 79 for child heart rate, for other condition
            if(heartAmp==0)
                heartAmp = rand()%400+150; //set low heart amplitude for other condition
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
