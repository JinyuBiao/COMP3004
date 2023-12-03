#ifndef PATIENTMANAGEMENTSYSTEM_H
#define PATIENTMANAGEMENTSYSTEM_H
#include "mainwindow.h"

class PatientManagementSystem: public MainWindow{
    Q_OBJECT
public:

    PatientManagementSystem(QWidget *parent  = nullptr);
    ~PatientManagementSystem();
    void waitingForPad();
    void padDetecting();
private:

};
#endif // PATIENTMANAGEMENTSYSTEM_H
