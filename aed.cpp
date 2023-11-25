#include "aed.h"

Aed::Aed()
{

}

Aed::~Aed()
{
}

bool Aed::detectShockable()//needs to add print statement after AedUI is done
{
    if(detectPad()){
        switch(detectPatientState()){
            case fibrillation:
                return(getAvgAmp()>100);
                break;
            case tachycardia:
                if(AdultPad)
                    return(patient.getHeartRate()>150);
                else
                    return(patient.getHeartRate()>200);
                break;
            default:
                return false;
                break;
        }
    }
    return false;
}

int Aed::getAvgAmp()
{
    int average = 0;
    for(int i : patient.getAmp()){
        average += i;
    }
    average = average/patient.getAmp().size();

    return average;
}

StateType Aed::detectPatientState()
{
    if(detectPad()){
        StateType state;
        if(patient.getHeartRate()==0){
            state = dead;
        }
        else if(AdultPad){
            if(patient.getHeartRate()>=101){
                state = tachycardia;
            }
            else
        }
    }
}

bool Aed::detectPad()
{
    return(connectedElectrode&&(ChildPad||AdultPad));
}
