#ifndef PATIENT_H
#define PATIENT_H

#include "defaultHeader.h"
class Patient{
public:
    Patient();
    int getHeartRate();
    PatientStateType getState();
    int getAmp();
    QString getCurrState();

    bool inContact();
    bool notChild();
    bool hasPad();

    ~Patient();
private:
    friend class PatientSimulator;

    void setAge();
    void setPad();
    void setStandClear();
    void setState();
    void setHeartData();
    void resetPatientStatus();

    int age;
    bool padPlaced;
    bool standClear;

    PatientStateType currState;

    int heartRate;
    int heartAmp;
};

#endif // PATIENT_H
