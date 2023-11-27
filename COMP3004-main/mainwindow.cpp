#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createPatient();

    mainProcessTimer = new QTimer(this);

    waitPadTime = 0;
    anaylzingTime = 0;
    cprTime = 0;
    aedWaiting = false;
    currStep = 1;

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
    connect(ui->padSelectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::padSelecting);
    connect(ui->padPlacedCheckBox, &QCheckBox::clicked, this, &MainWindow::placePad);
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
    setSimulateButtons(simulationEnabled);


    ui->createPatientButton->setEnabled(!operating);
    ui->padSelectionComboBox->setEnabled(!operating); //set pad selection disabled because once electrode is connected and device is turned on, it can not be unplugged
    ui->electrodeConnectedCheckBox->setEnabled(!operating);
    ui->statusBarQFrame->setVisible(operating);
    ui->cprBar->setVisible(operating);
    ui->ecgWaveGraph->setVisible(operating);
    ui->blackScreen->setVisible(!operating);
    ui->padLabel->setVisible(operating);
}

void MainWindow::togglePowerButton(bool checked)
{
    if(aed.getBattery() > 0.0){
        operating = checked;
        changeDeviceState();
        if(checked){
            aed.newPatient(patient);
        }
    }
    if(aedWaiting){//if aed is turned off while it is still in process, set aedWaiting to false and stop all processes
        aedWaiting = false;
        stopProcess();
    }
    ui->powerOnButton->setChecked(checked);
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
        if(ui->padSelectionComboBox->currentText()=="AdultPad"){
            aed.setAdultPad(true);
            aed.setChildPad(false);
            ui->padLabel->setPixmap(QPixmap(":/pic/electrodeAdultPadOn"));
        }
        else{
            aed.setAdultPad(false);
            aed.setChildPad(true);
            ui->padLabel->setPixmap(QPixmap(":/pic/electrodeChildPadOn"));
        }
    }
    else{
        ui->electrodeLabel->setText("Electrode not connected");
        ui->padLabel->setPixmap(QPixmap(":/pic/electrodePadOff"));
    }
}

void MainWindow::simulateFib()
{
    currStep = 0;
    aedWaiting = true;
    initializeMainTimer(mainProcessTimer);
    setSimulateButtons(false);
    ui->padSelectionComboBox->setEnabled(false);
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

    if(aed.selfCheck()){ //if selfcheck passed, the image on the first step button will be switched to the step1_light.svg by set the button to checked
        qDebug() << "starting process " << QString::number(currStep);
        stepImages[0]->setChecked(true); //this sets the first step button to checked, so the image will change, same for all other steps
        currStep++;
        t->start(2000);
    }
    //else()
        //t->disconnect();
}

void MainWindow::updateMainTimer()
{
    consumingBattery(1);

    switch(currStep){
        case 1://this finishes the first step (set first step button to unchecked) and start the second step
            stepImages[0]->setChecked(false);
            stepImages[1]->setChecked(true);
            qDebug() << "starting process " << QString::number(currStep);
            //if(nextStep)
                currStep++;
        break;
        case 2://this finishes the second step and start the third step
            stepImages[1]->setChecked(false);
            stepImages[2]->setChecked(true);
            qDebug() << "starting process " << QString::number(currStep);
            //if(nextStep)
                currStep++;
        break;
        case 3://this tries to finishes the third step (pad detection) and if finished, start the fourth step
            padDetecting();
        break;
        case 4://this shall try to finish the fourth step (heart analyzing step) and if finished, start the fifth step
            stepImages[3]->setChecked(false);
            stepImages[4]->setChecked(true);
            qDebug() << "starting process " << QString::number(currStep);
            //if(nextStep)
                currStep++;
        break;
        case 5://this shall try to finish the fifth step (cpr) and if finished, it shall determine patient's condition, to decide whether re run the heart analyzing step again or stop the process
            //if(nextStep)
                stepImages[4]->setChecked(false);
                qDebug() << "starting process " << QString::number(currStep);
                currStep++;
                aedWaiting = false; //going next step at step 5 meaning cpr is done and patient is healthy, the process shall terminates
            //otherwise currStep--, which means patient is not healthy and should continue doing shock and cpr, back to the anaylzing step (step 4) if patient still needs treatment
        break;
    }
    if(!aedWaiting){
        stopProcess();//aed not waiting meaning there is no process ongoing
        setSimulateButtons(true); //whenever aed is not in a treatment process, all scenario simulation buttons should be enabled
    }
}

void MainWindow::setSimulateButtons(bool state)
{
    ui->deadPatientButton->setEnabled(state);
    ui->fibrillationButton->setEnabled(state);
    ui->tachycardiaButton->setEnabled(state);
    ui->otherRhythmsButton->setEnabled(state);
}

void MainWindow::padSelecting(int index)
{
    if(aed.isConnected()){
        if(index == 0){
            aed.setAdultPad(true);
            aed.setChildPad(false);
            ui->padLabel->setPixmap(QPixmap(":/pic/electrodeAdultPadOn"));
        }
        else{
            aed.setAdultPad(false);
            aed.setChildPad(true);
            ui->padLabel->setPixmap(QPixmap(":/pic/electrodeChildPadOn"));
        }
    }
}

/*void MainWindow::initializeAedTimer(QTimer* aedTimer)
{
    aed.aedWaiting = true;
    aedTimer->start(1000);
}*/



void MainWindow::placePad()
{
    patient->setPad();
}

void MainWindow::waitingForPad()
{
    if(patient->hasPad() || waitPadTime == 2){
        waitPadTime = 0;
        stepImages[2]->setChecked(false);
        togglePowerButton(false);
    }
    else{
        waitPadTime++;
    }
}

void MainWindow::padDetecting()
{
    if(patient->hasPad()){
        stepImages[2]->setChecked(false);
        stepImages[3]->setChecked(true);
        qDebug() << "starting process " << QString::number(currStep);
        currStep++;
    }
    else{
        waitingForPad();
        qDebug() << "waiting for pad at process " << QString::number(currStep-1);
    }
}

void MainWindow::stopProcess()
{
    if(!operating && currStep <= 5){ //if users turn off device when the process (5 steps) is not over, uncheck the last step before turnning off
        stepImages[currStep-1]->setChecked(false);
        qDebug() << "stop at process " << QString::number(currStep-1);
    }
    mainProcessTimer->stop();
    mainProcessTimer->disconnect();
}
