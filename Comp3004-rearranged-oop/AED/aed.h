#ifndef AED_H
#define AED_H

#include "defaultHeader.h"
#include "aed_gui.h"
#include "cprHelper.h"
#include "patientSimulator.h"

class AED : public QObject{
public:
    AED();
    ~AED();

    void changeDeviceState();

    bool selfCheck();

private:
    QTimer* mainProcessTimer;
    QTimer* cprBarDropTimer;

    int currStep;
    int waitPadTime;
    int analyzingTime;
    int cprTime;
    int ecgTime;

    bool aedWaiting;
    bool nextStep;
    bool operating;

    void fillBattery();
    void setBattery(double);
    void setElectrode(bool);

    bool detectPad();
    bool detectShockable();
    bool isConnected();
    void connectElectrode(bool);


    void consumeBatteryInJ();
    void waitingForPad();
    void padDetecting();

    double getBattery();

    bool ChildPad;
    bool AdultPad;
    bool connected;

    int batteryLeftJ;
    double batteryLeftPercentage;


    AED_GUI gui;
    CprHelper* cprHelper;
    PatientSimulator* patientSimulator;

private slots:
    void togglePowerButton(bool);
    void updateMainTimer();

    void setChildPad(bool);
    void setAdultPad(bool);

    void changeBatteryLeftInPercentage(double);
    void changeBatteryLeftInJ(int);

    bool hasAdultPad();
    void padSelecting(int);
    void placePad();

};


#endif // AED_H
