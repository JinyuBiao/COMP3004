#ifndef AEDINTERFACE_H
#define AEDINTERFACE_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QDebug>
#include <QPushButton>

#include "patient.h"
#include "dataProcessor.h"
#include "qcustomplot.h"
#include "constantsHeader.h"


/* Class Purpose: The main window of the application, acts as the control of the application
 *
 * Data Members:
 *   DataProcessor dataprocessor: the processor for aed, used for self check and generating heart data
 *   Patient* patient: patient pointer
 *
 *   QTimer* mainProcessTimer: the timer for simulating the procedure of aed (step 1 to cpr step), once this timer is stopped, it means the scenario is finished and aed is waiting for next scenario
 *   QTimer* cprBarDropTimer: the timer for simulating cpr compression bar animation
 *   int currStep: indicating which step aed is at now, 1 indicates checking responsiveness, 2 is calling 911, 3 indicates pad placing, 4 is rhythm anaylzing and 5 is cpr period
 *   int waitPadTime: the timer count for waiting for pad
 *   int anaylzingTime: the timer count for anaylzing
 *   int cprTime: the timer count for cpr period
 *   int cprCount: the number of cpr performed during cpr period
 *   int previousCpr: the index of the cpr button pushed, 0 corresponds to the 0.5 inches cpr button, 1 corresponds to 1 inch button, 2 is 2 inches button
 *   double waveGraphX: the x value for updating the wave graph (e.g. 70 meaning adding a point that has x value 70)
 *   bool aedWorking: whether aed is in process or not (ie. in any of the 5 steps), turning on device does not set it to true, it is only set to true when aed is simulating scenarios
 *   bool operating: whether aed is turned on or not, turning on device set it to true, if aed run out of battery, it will be set to false
 *   StateType simulatedState: the current simulated state, it is used for checking the accuracy of aed analyzing (ie. showing whether patient is being touched)
 *   QVector<QPushButton*> stepImages: an array of buttons, each indicates the step image in ui (e.g. stepImages[0] is the check responsiveness image, stepImages[4] is the cpr image)
 *   QVector<QPushButton*> cprButtons: an array of cpr push buttons (e.g. cprButtons[0] is 0.5 inch button, cprButtons[2] is 2 inches button)
 *
 *
 *
 * Class Functions:
 *   void changeDeviceState(): enable or disable scenario pushbuttons and cpr buttons, and change the visiablity of ecg graph, battery bar and cpr push bar, etc.
 *   void consumingBattery(double): decrease the battery with a value
 *   void initializeMainTimer(): initialize the mainProcessTimer
 *   void setSimulateButtons(bool): set simulate scenario buttons to true or false
 *   void setCprButtons(bool): set cpr buttons to true or false
 *   void waitingForPad(): helper function for padDetecting()
 *   void padDetecting(): check if pad is connected and placed to patient at step 3
 *   void stopProcess(): set every buttons and labels (e.g. scenario buttons and ecg wave graph) to the state when device is not working, and set all timer counts to 0, aedWorking to false
 *   void startProcess(): start the working procedure of aed (ie. step 1 to cpr period)
 *   void startAnaylzing(): called at step 4 for generating heart data and updating ecg waveform
 *   void updatingEcg(double,double): draw a data on ecg graph
 *   void generateHeartData(): get the hearted data anaylzed by aed and draw it on ecg graph using updatingEcg() function
 *   void doCpr(): simulating cpr process
 *   void setPatientInfo(QString): updating the patient panel
 *
 *  slots functions:
 *   void fillBattery(): the slot function for clicking fill battery button (set battery to 100)
 *   void togglePowerButton(bool): for clicking power on button
 *   void changeBatteryLeft(double): change battery to a certain value
 *   void connectElectrode(bool): for clicking electrode connect checkbox, and change the state of whether device has a connected electrode
 *   void simulateFib(): for clicking simulate fibrillation button
 *   void simulateTach(): for clicking simulate tachycardia button
 *   void simulateDead(): for clicking simulate dead patient button
 *   void simulateOther(): for clicking simulate other rythms button
 *   void updateMainTimer(): update the step images in ui, and simulate anaylzing period, cpr period and waiting pad period
 *   void padSelecting(int): for choosing value in the combo box of pad selection (choose which pad to place)
 *   void placePad(bool): set aed adultpad or childpad in aed
 *   void changeAge(): change the age group of patient
 *   void cprPush(): for clicking cpr bar
 *   void cprBarDrop(): for simulating cpr bar drop
 *   void setPatientTouched(bool): for clicking touch patient check box
 */

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    Ui::MainWindow *ui;
    DataProcessor* dataProcessor;
    Patient* patient;

    QTimer* mainProcessTimer;
    QTimer* cprBarDropTimer;
    int currStep;
    int waitPadTime;//used for the pad dectection step
    int anaylzingTime;
    int cprTime;
    int cprCount;
    int shockCount;
    int previousCpr;
    int currCpr;
    double waveGraphX;

    bool aedWorking;//used for determine aed is in process or not
    bool operating;//aed is turned on or not

    StateType simulatedState;//used for contact se
    QVector<QPushButton*> stepImages;
    QVector<QPushButton*> cprButtons;

    void changeDeviceState();
    void consumingBattery(double);
    void initializeMainTimer();
    void setSimulateButtons(bool);
    void setCprButtons(bool);

    void waitingForPad();
    void padDetecting();

    void stopProcess();
    void startProcess();
    void startAnaylzing();
    void updatingEcg(double,double);
    void generateHeartData();
    void doCpr();
    void setPatientInfo(QString);
    void cprPrompt();

    //helper methods
    void initializeCounterValues();
    void initializeButtons();
    void initializeECGWaveGraph();
    void connectButtons();

    void disconnectElectrode();
    void connectedToAdultPad();
    void connectedToChildPad();
    void disconnectedPad();
    void detectSelectedPad();

    void determinePatientSurvival();

private slots:
    void togglePowerButton(bool);

    void fillBattery();
    void changeBatteryLeft(double);


    void simulateFib();
    void simulateTach();
    void simulateDead();
    void simulateOther();

    void updateMainTimer();
    void padSelecting(int);
    void placePad(bool);
    void connectElectrode(bool);

    void changeAge();
    void setPatientTouched(bool);
    void givingShock();

    void cprPush();
    void cprBarDrop();
    void cprBarDropHelper(int i);

};
#endif // AEDINTERFACE_H
