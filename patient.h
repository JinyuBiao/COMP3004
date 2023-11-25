#ifndef PATIENT_H
#define PATIENT_H

#include <QString>
#include <QVector>
#include <QtMath>

typedef enum {fibrillation, tachycardia, dead, healthy} StateType;

class Patient
{
    public:
        Patient(bool=true,bool=false,bool=true,StateType=dead,int=0);
        ~Patient();
        void setChild();
        void setPad();
        void setStandClear();
        void setState(StateType);
        void setHeartData();
        int getHeartRate();
        QVector<int>& getAmp();
    private:
        bool isAdult;
        bool padPlaced;
        //bool healthy;
        bool standClear;

        //bool cardiacArrhythmia;
        StateType currState;

        int heartRate; //in microvolts
        QVector<int> heartAmp; //heart amplitudes in an array
};

#endif // PATIENT_H
