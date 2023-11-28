#ifndef AED_H
#define AED_H

#include <QtGlobal>
#include <QObject>
#include <QTimer>
#include "patient.h"

class Aed : public QObject
{
    public:
        Aed();
        ~Aed();
        void fillBattery();
        void setBattery(double);
        void newPatient(Patient*);
        void setCPRdepth(int);
        void resetPatientStatus();
        void setElectrode(bool);

        bool selfCheck();
        bool detectPad();
        bool detectShockable();
        bool isConnected();
        bool hasAdultPad();

        double getBattery();
        int getCprDepth();


        StateType detectPatientState();
        QTimer* getTimer();

    public slots:
        void setChildPad(bool);
        void setAdultPad(bool);
        //void waitingForPad();

    private:
        double batteryLeft;

        int cprDepth;

        bool connected;	//default to false
        bool ChildPad;//default to false
        bool AdultPad;//default to false

        /*
        bool signalECG;				//defaults true
        bool functionalElectrode;	//default to true
        bool functionalMicroptocessor;		//default to true
        bool functionalSoftware;			//default to true
        bool functionalCompressionDepthDetector;	//default to true
        bool functionalVoicePrompt;			//default to true
        bool functionalVisualIndicator;		//default to true
        */

        Patient* patient;
        QTimer* timer;
};

#endif // AED_H
