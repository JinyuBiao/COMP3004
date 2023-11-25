#include "patient.h"

Patient::Patient(bool a, bool p, bool c, StateType d, int r):
    isAdult(a),padPlaced(p),standClear(c),currState(d),heartRate(r)
{}

Patient::~Patient()
{}

void Patient::setChild()
{
    isAdult = false;
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
            if(isAdult)
                heartRate = rand()%41+60; //generates number between 60 to 100 for adult heart rate
            else
                heartRate = rand()%71 + 80; //generate number between 80 to 150 for child heart rate
            break;
        case tachycardia:
            if(isAdult)
                heartRate = rand()%100 + 101; //generates number between 101 to 200 for adult heart rate
            else
                heartRate = rand()%100 + 151; //generate number between 151 to 250 for child heart rate
            for(int i=0; i<20; ++i){
                heartAmp.append(1000); //append to Amplitude array with amplitude 1000 for 20 times
            }
            break;
        case dead:
            heartRate = 0;
            heartAmp.clear();
            break;
        case healthy:
            if(isAdult){
                heartRate = rand()%41+60;
            }
            else{
                heartRate = rand()%71 + 80;
            }
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

QVector<int>& Patient::getAmp()
{
    return(heartAmp);
}
