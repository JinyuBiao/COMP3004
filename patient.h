#ifndef PATIENT_H
#define PATIENT_H

#include <QString>
#include <QtMath>

typedef enum {fibrillation, tachycardia, dead, healthy, other} StateType;

class Patient
{
    public:
        Patient(bool=true,bool=false,bool=true,StateType=dead,int=0);
        ~Patient();
        void changeAge();
        void setPad();
        void setStandClear();
        void setState(StateType);
        void setHeartData();
        int getHeartRate();
        bool notInContact();
        bool notChild();
        bool hasPad();
        StateType getState();
        int getAmp();
    private:
        bool isAdult;
        bool padPlaced;
        //bool healthy;
        bool standClear;

        //bool cardiacArrhythmia;
        StateType currState;

        int heartRate; //in microvolts
        int heartAmp; //heart amplitudes in an array
};

#endif // PATIENT_H
