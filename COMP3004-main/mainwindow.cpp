#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    patient = new Patient();

    mainProcessTimer = new QTimer(this);

    anaylzingTime = 0;
    cprTime = 0;

    //deviced initially set to off
    operating = false;
    changeDeviceState();

    stepImages.push_back(ui->step1);
    stepImages.push_back(ui->step2);
    stepImages.push_back(ui->step3);
    stepImages.push_back(ui->step4);
    stepImages.push_back(ui->step5);
    for(int i=0; i<5; ++i){
        stepImages[i]->setDisabled(true);
    }


    ui->shockButton->setEnabled(false);


    connect(ui->createPatientButton, &QPushButton::released, this, &MainWindow::createPatient);
    connect(ui->powerOnButton, &QPushButton::clicked, this, &MainWindow::togglePowerButton);
    connect(ui->fillBatteryButton, &QPushButton::released, this, &MainWindow::fillBattery);
    connect(ui->electrodeConnectedCheckBox, &QCheckBox::clicked, this, &MainWindow::connectElectrode);
    connect(ui->batterySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::changeBatteryLeft);
    connect(ui->fibrillationButton, &QPushButton::released, this, &MainWindow::simulateFib);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mainProcessTimer;
    delete patient;
}

void MainWindow::createPatient()
{
    delete patient;
    patient = new Patient();
    //(patient->notChild()) ? ui->patientInfoAge->setText("Adult") : ui->patientInfoAge->setText("Child");
    ui->patientInfoAge->setText("Adult");
}

void MainWindow::changeDeviceState()
{
    bool simulationEnabled = operating&&ui->electrodeConnectedCheckBox->isChecked();
    ui->deadPatientButton->setEnabled(simulationEnabled);
    ui->fibrillationButton->setEnabled(simulationEnabled);
    ui->tachycardiaButton->setEnabled(simulationEnabled);
    ui->otherRhythmsButton->setEnabled(simulationEnabled);

    ui->createPatientButton->setEnabled(!operating);
    ui->padSelectionComboBox->setEnabled(!operating); //set pad selection disabled because once electrode is connected and device is turned on, it can not be unplugged
    ui->electrodeConnectedCheckBox->setEnabled(!operating);
    ui->statusBarQFrame->setVisible(operating);
    ui->cprBar->setVisible(operating);
    ui->ecgWaveGraph->setVisible(operating);
    ui->blackScreen->setVisible(!operating);
}

void MainWindow::togglePowerButton(bool checked)
{
    if(aed.getBattery() > 0.0){
        operating = checked;
        changeDeviceState();
    }
    else{
        ui->powerOnButton->setChecked(false);
    }

    if(currStep != -1){
        stepImages[currStep]->setChecked(false);
        currStep = -1;
    }
}

void MainWindow::changeBatteryLeft(double batteryLeft)
{
    if(batteryLeft >= 0.0 && batteryLeft <= 100.0){
        if(batteryLeft == 0 && operating){
            togglePowerButton(false);
            ui->powerOnButton->setChecked(false);
        }
        aed.setBattery(batteryLeft);
        ui->batterySpinBox->setValue(batteryLeft);
        ui->batteryLevelBar->setValue(batteryLeft);

        if(batteryLeft <= 20){
            ui->batteryLevelBar->setStyleSheet("QProgressBar { selection-background-color: red; background-color: gray;}");
        }
        else{
            ui->batteryLevelBar->setStyleSheet("QProgressBar { selection-background-color: green; background-color: gray;}");
        }
    }
}

void MainWindow::fillBattery()
{
    changeBatteryLeft(100.0);
}

void MainWindow::consumingBattery(double consumption)
{
    double currBattery = aed.getBattery();
    currBattery = qMax((currBattery - consumption),0.0);
    changeBatteryLeft(currBattery);
}

void MainWindow::connectElectrode(bool connection)
{
    aed.setElectrode(connection);
    if(connection){
        ui->electrodeLabel->setText("Electrode connected");
    }
    else{
        ui->electrodeLabel->setText("Electrode not connected");
    }
}

void MainWindow::simulateFib()
{
    initializeMainTimer(mainProcessTimer);
}

void MainWindow::simulateTach()
{

}

void MainWindow::simulateDead()
{

}

void MainWindow::simulateOther()
{

}

void MainWindow::initializeMainTimer(QTimer* t)
{
    connect(t, &QTimer::timeout, this, &MainWindow::updateMainTimer);
    currStep = 1;
    t->start(2000);
}

void MainWindow::updateMainTimer()
{
    if(currStep == 6 || !operating){
        if(currStep <= 5)
            stepImages[currStep-1]->setChecked(false);
        else if(currStep == 6)
            stepImages[4]->setChecked(false);
        mainProcessTimer->stop();
        currStep = 1;
    }
    else{
        consumingBattery(1);

        switch(currStep){
            case 1:
                stepImages[0]->setChecked(true);
                currStep++;
            break;
            case 2:
                stepImages[0]->setChecked(false);
                stepImages[1]->setChecked(true);
                currStep++;
            break;
            case 3:
                stepImages[1]->setChecked(false);
                stepImages[2]->setChecked(true);
                currStep++;
            break;
            case 4:
                stepImages[2]->setChecked(false);
                stepImages[3]->setChecked(true);
                currStep++;
            break;
            case 5:
                stepImages[3]->setChecked(false);
                stepImages[4]->setChecked(true);
                currStep++;
            break;
        }
    }

}
