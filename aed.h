#ifndef AED_H
#define AED_H

#include "patient.h"

class Aed
{
    public:
        Aed();
        ~Aed();
        bool selfCheck();
        void analyze();
        bool detectPad();
        bool detectShockable();
        StateType detectPatientState();
        int getAvgAmp();
    private:
        int batteryLeft;
        bool doingCPR;
        int cprDepth;

        bool connectedElectrode;	//default to false
        bool ChildPad;//default to false
        bool AdultPad;//default to false

        bool signalECG;				//defaults true
        bool functionalElectrode;	//default to true
        bool functionalMicroptocessor;		//default to true
        bool functionalSoftware;			//default to true
        bool checkCPR;				//default to false
        bool functionalCompressionDepthDetector;	//default to true
        bool functionalVoicePrompt;			//default to true
        bool functionalVisualIndicator;		//default to true

        Patient patient;
};

#endif // AED_H
