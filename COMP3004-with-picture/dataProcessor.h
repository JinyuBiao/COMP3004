#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QtGlobal>
#include <QObject>
#include <QTimer>
#include "patient.h"

/* Purpose of Class: Simulate aed functions
 *
 * Data Members:
 *  double batteryLeft: current battery
 *  int heartRate: analyzed heart rate
 *  int heartAmp: analyzed heart amp
 *  bool connected:	if it has electrode connected
 *  bool ChildPad: if the connected electrode pad is Child pad
 *  bool AdultPad: if the connected electrode pad is Adult pad
 *  StateType detectedState: the detected rhythm, used for generating heart data
 *  Patient* patient: the patient
 *
 * Class Functions:
 *  Getters and Setters
 *  void fillBattery(): set battery to 100
 *  void setBattery(double): change battery to a certain value
 *  void newPatient(Patient*): set patient
 *  void setConnected(bool): set connection of electrode
 *  void setHeartData(): generate heart data base on detected heart rhythm
 *  void clearHeartData(): clear last analyzed data (before cpr)
 *
 *  bool selfCheck(): check battery level and electrode connection
 *  bool detectPad(): detect if there is a electrode connected and also pad placed on patient
 *  bool detectShockable(): determine if detected rhythm is shockable
 *  bool isConnected(): getter of whether there is electrode connection
 *  bool hasAdultPad(): getter of whether the pad used is adultpad
 */

class DataProcessor
{
    public:
        DataProcessor();
        ~DataProcessor();
        void fillBattery();
        void setBattery(double);
        void newPatient(Patient*);
        void setConnected(bool);
        void setHeartData();
        void clearHeartData();
        void setDetectedState(StateType);

        bool selfCheck();
        bool detectPad();
        bool detectShockable();
        bool isConnected();
        bool hasAdultPad();

        double getBattery();
        int getAmp();
        int getHeartRate();


        StateType getDetectedState();
        QString getDetectedStateString();
    public slots:
        void setChildPad(bool);
        void setAdultPad(bool);
        //void waitingForPad();

    private:
        double batteryLeft;

        int heartRate;
        int heartAmp;

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

        StateType detectedState;
        Patient* patient;
};

#endif // DATAPROCESSOR_H
