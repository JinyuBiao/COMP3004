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
            for(int i=0; i<20; ++i){
                heartAmp.append((rand()%2000)+1); //append to Amplitude array with a random number between 1 to 2000 for 20 times
            }
            heartRate = (isAdult) ? rand()%41+60 : rand()%41 + 80; //generates number between 60 to 100 for adult heart rate otherwise between 80 to 120 for child heart rate
            break;
        case tachycardia:
            heartRate = (isAdult) ? rand()%100 + 101 : rand()%100 + 121; //generates number between 101 to 200 for adult heart rate, otherwise between 121 to 220 for child heart rate
            for(int i=0; i<20; ++i){
                heartAmp.append(1000); //append to Amplitude array with amplitude 1000 for 20 times
            }
            break;
        case dead:
            heartRate = 0;
            heartAmp.clear();
            break;
        case healthy:
            heartRate = (isAdult) ? rand()%41+60 : rand()%71 + 80;
            for(int i=0; i<20; ++i){
                heartAmp.append(1000); //append to Amplitude array with amplitude 1000 for 20 times
            }
            break;
        case other:
            heartRate = (isAdult) ? rand()%59+1 : rand()%79 + 1;
            for(int i=0; i<20; ++i){
                heartAmp.append(1000); //append to Amplitude array with amplitude 1000 for 20 times
            }
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

QVector<int>& Patient::getAmp()
{
    return heartAmp;
}

StateType Patient::getState()
{
    return currState;
}
