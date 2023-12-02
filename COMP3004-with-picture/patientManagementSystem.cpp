#include "patientManagementSystem.h"

PatientManagementSystem::PatientManagementSystem(QWidget *parent)
    :MainWindow(parent)
    , ui(new Ui::MainWindow){

}

PatientManagementSystem::~PatientManagementSystem(){}

void PatientManagementSystem::waitingForPad(){
    //whenever aed has a pad detected, or aed does not detect any pad after WAIT_PAD_TIME elpased, aed should turn off
    if(waitPadTime >= WAIT_PAD_TIME){
        waitPadTime = 0;
        stepImages[2]->setChecked(false);//set step 3 (detect pad) image to gray light
        aedWorking = false;
        if(!dataProcessor->detectPad()){
            qDebug() << "Waiting time reached. No action was performed. System timed out.";
            operating = false;
        }
    }
    else{
        waitPadTime++;
        qDebug() << "Current waiting time is " << QString::number(waitPadTime);
    }
}

void PatientManagementSystem::padDetecting(){
    //whenever pad is detected at step 3, on to the next step by adding 1 to currStep
    if(dataProcessor->detectPad()){
        stepImages[2]->setChecked(false);
        stepImages[3]->setChecked(true);
        qDebug() << "start detecting pad at process " << QString::number(currStep);
        currStep++;
    }
    else{
        waitingForPad();
        qDebug() << "waiting for pad at process " << QString::number(currStep-1);
    }
}
