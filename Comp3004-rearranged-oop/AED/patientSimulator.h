#ifndef PATIENTSIMULATOR_H
#define PATIENTSIMULATOR_H

#include "defaultHeader.h"
#include "patient.h"

class PatientSimulator{
public:
    PatientSimulator();
    ~PatientSimulator();

    void newPatient(Patient*);
    void generateHeartDate();
    void setPatientInfo(QString);

    void simulateFib();
    void simulateTach();
    void simulateDead();
    void simulateOther();
private:
    Patient* patient;
};

#endif // PATIENTSIMULATOR_H
