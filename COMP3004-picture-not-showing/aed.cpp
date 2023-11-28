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
    connected = connection;
}

bool Aed::hasAdultPad()
{
    return (AdultPad);
}

bool Aed::selfCheck()
{
    if(batteryLeft<15){
        //aed.lowBatteryMessage()
        qInfo("low battery, turning on failed");
    }
    else if(!connected){
        //aed.noElectrodeMessage()
        qInfo("no electrode connected, turning on failed");
    }
    return (batteryLeft>=15&&connected);
}

bool Aed::detectPad()
{
    return(connected&&(ChildPad||AdultPad));
}

bool Aed::detectShockable()//needs to add AedUI functions for printing after AedUI is done
{
    bool shockable = false;
    if(detectPad()){
        if(detectPatientState()==fibrillation||detectPatientState()==tachycardia){
            shockable = true;
        }
        else if(detectPatientState()==dead){
            //AedUI methods to warning a flatline or no signal detected on console goes here:

        }
        else{
            //AedUI methods to state that patient is healthy on console goes here:

        }
    }
    else{
        //AedUI methods to state that there is no pad connected on console goes here:


    }

    return shockable;
}

bool Aed::isConnected()
{
    return connected;
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
    return(state);
}

QTimer* Aed::getTimer()
{
    return timer;
}
