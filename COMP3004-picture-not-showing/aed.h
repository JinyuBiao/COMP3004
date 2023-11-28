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
        //constants
            const int ENERGY_J_1 = 120;
            const int ENERGY_J_2 = 150;
            const int ENERGY_J_3 = 200;

            const int SEC_TO_MS = 1000;
            const int DEFAULT_CHARGE_TIME_SEC = 9;
            const int CPR_RATE_MIN = 60;
            const int CPR_RATE_MAX = 100;

            const float DEPTH_CHECK_MIN_CM = 1.9;
            const float DEPTH_CHECK_MAX_CM = 8.9;

            const int FIBRILLATION_ABOVE_AVG_AMPLITUDE = 100;
            const int TACHYCARDIA_ADULT_ABOVE_BPM = 150;
            const int TACHYCARDIA_PEDIATRIC_ABOVE_BPM = 200;

            //variables
            bool electrodeConnection;	//default to false


            double batteryLeft;

            int cprDepth;

            bool connected;	//default to false
            bool ChildPad;//default to false
            bool AdultPad;//default to false

            bool signalECG = true;                      //defaults true
            bool functionalElectrode = true;            //default to true
            bool functionalMicroptocessor = true;		//default to true
            bool functionalSoftware = true;             //default to true
            bool functionalCompressionDepthDetector = true;	//default to true
            bool functionalVoicePrompt = true;			//default to true
            bool functionalVisualIndicator = true;		//default to true


            bool anaylzing;         //indicates whether aed is detecting rhythms
            bool cprPeriod;

            //default settings from AED plus specifications.
            int Energy = ENERGY_J_1;
            //will be selected based on required shock


        Patient* patient;
        QTimer* timer;
};

#endif // AED_H
