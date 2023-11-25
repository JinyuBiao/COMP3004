#ifndef AED_H
#define AED_H

#include <QtGlobal>
#include "patient.h"

class Aed
{
    public:
        Aed();
        ~Aed();
        void drainBattery(int);
        void newPatient(Patient&);
        void touchPatient();
        bool selfCheck();
        bool detectPad();
        bool detectShockable();
        bool checkCPR();
        double getBattery();
        int getCprDepth();
        StateType detectPatientState();
    private:
        double batteryLeft;
        bool doingCPR;
        int cprDepth;

        bool connectedElectrode;	//default to false
        bool ChildPad;//default to false
        bool AdultPad;//default to false

        bool signalECG;				//defaults true
        bool functionalElectrode;	//default to true
        bool functionalMicroptocessor;		//default to true
        bool functionalSoftware;			//default to true
        bool functionalCompressionDepthDetector;	//default to true
        bool functionalVoicePrompt;			//default to true
        bool functionalVisualIndicator;		//default to true

        Patient patient;
};

#endif // AED_H
