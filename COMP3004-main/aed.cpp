#include "aed.h"

Aed::Aed()
{
    timer = new QTimer(this);
    batteryLeft = 100;
}

Aed::~Aed()
{
    delete timer;
}

void Aed::setBattery(double i)
{
    batteryLeft = i;
}

void Aed::fillBattery()
{
    batteryLeft = 100;
}

void Aed::newPatient(Patient* p)
{
    patient = p;
}

void Aed::setCPRdepth(int d)
{
    cprDepth = d;
}

void Aed::doCPR()
{
    if(detectPatientState()==tachycardia||detectPatientState()==fibrillation||detectPatientState()==other){
        cprPeriod = true;
        //AedUI methods prompting user to do CPR on console goes here:

    }
    else if(detectPatientState()==dead){
        cprPeriod = false;
        //AedUI methods giving flatline warnning:

    }
    else{
        cprPeriod = false;
        //AedUI methods giving healthy patient warnning:

    }
}

void Aed::startAnaylzing()
{
    anaylzing = true;
}

void Aed::noActivity()
{
    anaylzing = false;
    cprPeriod = false;
}

void Aed::setAdultPad(bool placed)
{
    AdultPad = placed;
}

void Aed::setChildPad(bool placed)
{
    ChildPad = placed;
}

void Aed::setElectrode(bool connection)
{
    electrodeConnection = connection;
}

bool Aed::selfCheck()
{
    return (batteryLeft>=5&&electrodeConnection);
}

bool Aed::detectPad()
{
    return(electrodeConnection&&(ChildPad||AdultPad));
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

bool Aed::isAnaylzing()
{
    return anaylzing;
}

double Aed::getBattery()
{
    return batteryLeft;
}

int Aed::getCprDepth()
{
    return cprDepth;
}

StateType Aed::detectPatientState()
{
    StateType state = dead;
    if(detectPad()){
        if(patient->notInContact()){
            state = patient->getState();
        }
        else{
            state = healthy;
        }
    }
    //AedUI methods to print detected patient's state on console goes here:

    return(state);
}

QTimer* Aed::getTimer()
{
    return timer;
}
