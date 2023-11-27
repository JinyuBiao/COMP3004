#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createPatient();

    ui->ecgWaveGraph->addGraph();
    ui->ecgWaveGraph->xAxis->setRange(0,50);
    ui->ecgWaveGraph->yAxis->setRange(0,2100);
    ui->ecgWaveGraph->xAxis->setVisible(false);
    ui->ecgWaveGraph->yAxis->setVisible(false);

    mainProcessTimer = new QTimer(this);

    waitPadTime = 0;
    anaylzingTime = 0;
    cprTime = 0;
    ecgTime = 0.0;
    aedWaiting = false;
    currStep = -1;

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
    connect(ui->padSelectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::padSelecting);
    connect(ui->padPlacedCheckBox, &QCheckBox::clicked, this, &MainWindow::placePad);
    connect(ui->changeAgeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changeAge);

    connect(ui->fibrillationButton, &QPushButton::released, this, &MainWindow::simulateFib);
    connect(ui->tachycardiaButton, &QPushButton::released, this, &MainWindow::simulateTach);
    connect(ui->deadPatientButton, &QPushButton::released, this, &MainWindow::simulateDead);
    connect(ui->otherRhythmsButton, &QPushButton::released, this, &MainWindow::simulateOther);
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
    if(checked){
        if(aed.selfCheck()){
            operating = checked;
            changeDeviceState();
            aed.newPatient(patient);
            qDebug() << "turning on ";
        }
    }
    else{
        operating = false;
        changeDeviceState();
        if(aedWaiting){//if aed is turned off while it is still in process, set aedWaiting to false and stop all processes
            aedWaiting = operating;
            stopProcess();
        }
        qDebug() << "turning off ";
    }
    ui->powerOnButton->setChecked(operating);
}

void MainWindow::changeBatteryLeft(double batteryLeft)
{
    if(batteryLeft >= 0.0 && batteryLeft <= 100.0){
        if(batteryLeft <= 0 && operating){
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
    patient->setState(fibrillation);
    startProcess();
}

void MainWindow::simulateTach()
{
    patient->setState(tachycardia);
    startProcess();
}

void MainWindow::simulateDead()
{
    patient->setState(dead);
    startProcess();
}

void MainWindow::simulateOther()
{
    patient->setState(other);
    startProcess();
}

void MainWindow::initializeMainTimer(QTimer* t)
{
    connect(t, &QTimer::timeout, this, &MainWindow::updateMainTimer);
    qDebug() << "starting process " << QString::number(currStep);
    stepImages[0]->setChecked(true); //this sets the first step button to checked, so the image will change, same for all other steps
    updatingEcg(1,1000);
    currStep++;
    t->start(2000);
}

void MainWindow::updateMainTimer()
{

    consumingBattery(1);

    if(aed.getBattery()<=0.0){
        aedWaiting = false;
    }
    switch(currStep){
        case 1://this finishes the first step (set first step button to unchecked) and start the second step
            stepImages[0]->setChecked(false);
            stepImages[1]->setChecked(true);
            qDebug() << "starting process " << QString::number(currStep);
            updatingEcg(10,1000);
            currStep++;
        break;
        case 2://this finishes the second step and start the third step
            stepImages[1]->setChecked(false);
            stepImages[2]->setChecked(true);
            qDebug() << "starting process " << QString::number(currStep);
            updatingEcg(10,1000);
            currStep++;
        break;
        case 3://this tries to finishes the third step (pad detection) and if finished, start the fourth step
            padDetecting();
        break;
        case 4://this shall try to finish the fourth step (heart analyzing step) and if finished, start the fifth step
            startAnaylzing();
            qDebug() << "doing process " << QString::number(currStep);
        break;
        case 5://this shall try to finish the fifth step (cpr) and if finished, it shall determine patient's condition, to decide whether re run the heart analyzing step again or stop the processif
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
        qDebug() << "stop at process " << QString::number(currStep-1);
        if(!operating){
            togglePowerButton(false);
        }
        setSimulateButtons(operating); //whenever aed is not in a treatment process, all scenario simulation buttons should be enabled
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

void MainWindow::placePad()
{
    patient->setPad();
}

void MainWindow::waitingForPad()
{
    if(patient->hasPad() || waitPadTime >= 2){
        waitPadTime = 0;
        stepImages[2]->setChecked(false);
        aedWaiting = false;
        if(!patient->hasPad()){
            operating = false;
        }
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
        qDebug() << "starting detecting pad at process " << QString::number(currStep);
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
    }
    mainProcessTimer->stop();
    mainProcessTimer->disconnect();
    ui->ecgWaveGraph->xAxis->setRange(0, 50);
    ui->ecgWaveGraph->graph(0)->data()->clear();
    ui->ecgWaveGraph->replot();
    ecgTime = 0;
}

void MainWindow::startProcess()
{
    if(aed.selfCheck()){
        currStep = 0;
        aedWaiting = true;
        initializeMainTimer(mainProcessTimer);
        setSimulateButtons(false);
        ui->padSelectionComboBox->setEnabled(false);
        ui->changeAgeComboBox->setEnabled(false);
    }
}

void MainWindow::changeAge()
{
    patient->changeAge();
    if(patient->notChild())
        ui->patientInfoAge->setText("Adult");
    else
        ui->patientInfoAge->setText("Child");
}

void MainWindow::startAnaylzing()
{
    if(anaylzingTime < 7){
        if(!patient->hasPad())
            patient->setState(dead);
        generateHeartData();
        //code for displaying ecg goes here:
        anaylzingTime++;
    }
    else if(anaylzingTime == 7 && !aed.detectShockable()){
        qDebug() << "no shock needed, on to the next step";
        stepImages[3]->setChecked(false);
        stepImages[4]->setChecked(true);
        anaylzingTime = 0;
        currStep++;
    }
    else if(!ui->shockButton->isChecked()){
        ui->shockButton->setEnabled(true);
        generateHeartData();
        qDebug() << "waiting for delivering shock";
    }
    else{
        double newRemainingBattery = (aed.hasAdultPad()) ? qMax(aed.getBattery() - 10, 0.0) : qMax(aed.getBattery() - 5, 0.0);
        changeBatteryLeft(newRemainingBattery);
        ui->shockButton->setChecked(false);
        ui->shockButton->setEnabled(false);
        stepImages[3]->setChecked(false);
        stepImages[4]->setChecked(true);
        anaylzingTime = 0;
        currStep++;
    }
}

void MainWindow::updatingEcg(double x, double y)
{
    ecgTime += x;
    ui->ecgWaveGraph->graph(0)->addData(ecgTime,y);
    if(ecgTime > 50){
        ui->ecgWaveGraph->xAxis->setRange(ecgTime-50, ecgTime+10);
    }
    ui->ecgWaveGraph->replot();
}

void MainWindow::generateHeartData()
{
    patient->setHeartData();

    if(patient->getState()==tachycardia || patient->getState()==other){//make graph resembles tachycardia
        updatingEcg(2,patient->getAmp()+1000);
        updatingEcg(2,1000-patient->getAmp());
        updatingEcg(2,1000);
        updatingEcg(220/patient->getHeartRate(),1000);
    }
    else if(patient->getState()==fibrillation){ //make graph resembles fibrillation
        double randomInvertal = (rand()%9+4)/(rand()%3+1);
        updatingEcg(randomInvertal,patient->getAmp()+1000);
        updatingEcg(randomInvertal,patient->getAmp());
    }
    else{
        updatingEcg(10,1000); //flatline for dead patient
    }
}
