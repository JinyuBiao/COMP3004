#include "aed.h"

Aed::Aed()
{
}

Aed::~Aed()
{
}

void Aed::drainBattery(int i)
{
    batteryLeft -= qMax((batteryLeft - i), 0.0);
}

void Aed::newPatient(Patient& p)
{
    patient = p;
}

void Aed::touchPatient()
{
    patient.setStandClear();
}

bool Aed::selfCheck()
{

}

bool Aed::detectPad()
{
    return(connectedElectrode&&(ChildPad||AdultPad));
}

bool Aed::detectShockable()//needs to add AedUI functions for printing after AedUI is done
{
    bool shockable = false;
    if(detectPad()){
        if(detectPatientState()==fibrillation||detectPatientState()==tachycardia){
            shockable = true;
        }
    }
    //AedUI methods to print text on console goes here:

    return shockable;
}

bool Aed::checkCPR()
{
    bool cpr = (detectPatientState()==tachycardia||detectPatientState()==fibrillation) ? true : false;
    //AedUI methods to print text on console goes here:

    return cpr;
}

StateType Aed::detectPatientState()
{
    StateType state = dead;
    if(detectPad()){
        if(patient.notInContact()){
            state = patient.getState();
        }
        else{
            state = healthy;
        }
    }
    //AedUI methods to print text on console goes here:

    return(state);
}

double Aed::getBattery()
{
    return batteryLeft;
}

int Aed::getCprDepth()
{
    return cprDepth;
}
