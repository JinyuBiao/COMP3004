#include "aed.h"

Aed::Aed()
{
    heartAmp = 0;
    heartRate = 0;
    batteryLeft = 100;
}

Aed::~Aed()
{
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

void Aed::setAdultPad(bool placed)
{
    AdultPad = placed;
}

void Aed::setChildPad(bool placed)
{
    ChildPad = placed;
}

void Aed::setConnected(bool connection)
{
    connected = connection;
}

void Aed::setHeartData()
{
    switch(detectedState){
        case fibrillation:
            heartAmp = ((rand()%1000)+1);//generate random heart amplitude between 1 to 1000, since it is realistic for fibrillation
            if(heartRate==0)
                heartRate = 1; //assume heart rate for fibrillation is 1 (heart rate for fibrillation is actually impossible to detect)
            break;
        case tachycardia:
            if(heartRate==0)
                heartRate = (AdultPad) ? rand()%100 + 101 : rand()%100 + 121; //generates heart rate between 101 to 200 for adult heart rate, otherwise between 121 to 220 for child heart rate, if haven't already
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
            heartRate = (AdultPad) ? 80 : 100; //generates 80 for adult heart rate otherwise between 100 for child heart rate
            heartAmp = 800; //generate heart amplitude 800 if haven't already
            break;
        case other:
            if(heartRate==0)
                heartRate = (AdultPad) ? rand()%30+1 : rand()%50 + 1; //generates low heart rate which is 1 to 59 for adult heart rate otherwise between 1 to 79 for child heart rate, for other condition
            if(heartAmp==0)
                heartAmp = rand()%400+150; //set low heart amplitude for other condition
            break;
    }
}

void Aed::clearHeartData()
{
    heartAmp = 0;
    heartRate = 0;
}

void Aed::setDetectedState(StateType state)
{
    detectedState = state;
}

bool Aed::hasAdultPad()
{
    return (AdultPad);
}

bool Aed::selfCheck()
{
    bool checkResult = batteryLeft>=15&&connected;
    if(checkResult){
        qInfo("self test passed");
    }
    else{
        if(batteryLeft<15){
            qInfo("low battery, self test failed, aed can not work! Please fill the battery.");
        }
        else{
            qInfo("no electrode connected, self test failed, aed can not work");
        }
    }
    return (checkResult);
}

bool Aed::detectPad()
{
    return(connected&&(ChildPad||AdultPad));
}

bool Aed::detectShockable()//needs to add AedUI functions for printing after AedUI is done
{
    bool shockable = false;
    if(detectPad()){
        if(detectedState==fibrillation||detectedState==tachycardia){
            shockable = true;
        }
        else if(detectedState==dead){
            //AedUI methods to warning a flatline or no signal detected on console goes here:
            qInfo("Patient has flatlined or no signal detected, shock is disabled");
            shockable = false;

        }
        else{
            //AedUI methods to state that patient is healthy on console goes here:
            qInfo("Patient is healthy, a shock is not required.");
            shockable = false;
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

int Aed::getHeartRate()
{
    return heartRate;
}

int Aed::getAmp()
{
    return heartAmp;
}

StateType Aed::getDetectedState()
{
    return detectedState;
}

QString Aed::getDetectedStateString()
{
    switch(detectedState){
        case fibrillation:
            return("ventricular fibrillation");
        case tachycardia:
            return("ventricular tachycardia");
        case other:
            return("other conditions");
        case dead:
            return("flatlined");
        case healthy:
            return("healthy");
        default:
            return("unknown");
    }
}
