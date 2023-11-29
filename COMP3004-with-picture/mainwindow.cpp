#include "mainwindow.h"
#include "ui_mainwindow.h"

#define ANALYZING_TIME 4
#define CPR_TIME 10

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    patient = new Patient();
    aed.newPatient(patient);
    ui->patientInfoAge->setText("Adult");

    ui->ecgWaveGraph->setBackground(QColor(0,0,0));
    ui->ecgWaveGraph->addGraph();
    ui->ecgWaveGraph->graph()->setPen(QPen(Qt::red));
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
    cprCount = 0;
    previousCpr = -1;

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

    cprButtons.push_back(ui->cprNoneDetectablePush);
    cprButtons.push_back(ui->cprPushHarder);
    cprButtons.push_back(ui->cprGoodPush);

    ui->shockButton->setEnabled(false);
    ui->selfTestLight->setVisible(false);

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
    connect(ui->cprNoneDetectablePush, &QPushButton::clicked, this, &MainWindow::cprPush);
    connect(ui->cprPushHarder, &QPushButton::clicked, this, &MainWindow::cprPush);
    connect(ui->cprGoodPush, &QPushButton::clicked, this, &MainWindow::cprPush);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mainProcessTimer;
    delete cprBarDropTimer;
    delete patient;
}

void MainWindow::changeDeviceState()
{
    bool simulationEnabled = operating;
    bool cprEnabled = operating&&(currStep == 5);
    setSimulateButtons(simulationEnabled);
    setCprButtons(cprEnabled);

    if(operating)
        ui->selfTestLight->setChecked(aed.selfCheck());
    ui->padSelectionComboBox->setEnabled(!operating); //set pad selection disabled because once electrode is connected and device is turned on, it can not be unplugged
    ui->statusBarQFrame->setVisible(operating);
    ui->cprBar->setVisible(operating);
    ui->ecgWaveGraph->setVisible(operating);
    ui->blackScreen->setVisible(!operating);
    ui->padLabel->setVisible(operating);
}

void MainWindow::togglePowerButton(bool checked)
{
    if(checked){
        if(aed.getBattery()>=0.0){
            operating = true;
            ui->selfTestLight->setVisible(true);
            changeDeviceState();
            qDebug() << "turning on ";
        }
    }
    else{
        operating = false;
        ui->selfTestLight->setVisible(false);
        changeDeviceState();
        if(aedWaiting){//if aed is turned off while it is still in process, set aedWaiting to false and stop all processes
            aedWaiting = operating;
            waitPadTime = 0; //reset pad waiting time
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
            qInfo("Battery low, please fill the battery in time!");
            ui->batteryLevelBar->setStyleSheet("QProgressBar { selection-background-color: red; background-color: gray;}");
        }
        else{
            ui->batteryLevelBar->setStyleSheet("QProgressBar { selection-background-color: green; background-color: gray;}");
        }
    }
}

void MainWindow::fillBattery()
{
    qInfo("Battery successfully filled");
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
            ui->padLabel->setPixmap(QPixmap(":/electrode/electrodeAdultPadOn"));
        }
        else{
            aed.setAdultPad(false);
            aed.setChildPad(true);
            ui->padLabel->setPixmap(QPixmap(":/electrode/electrodeChildPadOn"));
        }
    }
    else{
        aedWaiting = false;
        ui->electrodeLabel->setText("Electrode not connected");
        ui->padLabel->setPixmap(QPixmap(":/electrode/electrodePadOff"));
    }
    if(!aedWaiting && operating)//only updates self test light when aed is not in process and aed is on
        ui->selfTestLight->setChecked(aed.selfCheck());
}

void MainWindow::simulateFib()
{
    patient->setState(fibrillation);
    ui->currScenarioLabel->setText("Ventricular Fibrillation");
    ui->currScenarioLabel->setAlignment(Qt::AlignCenter);
    startProcess();
}

void MainWindow::simulateTach()
{
    patient->setState(tachycardia);
    ui->currScenarioLabel->setText("Ventricular Tachycardia");
    ui->currScenarioLabel->setAlignment(Qt::AlignCenter);
    startProcess();
}

void MainWindow::simulateDead()
{
    patient->setState(dead);
    ui->currScenarioLabel->setText("Faltlined Patient");
    ui->currScenarioLabel->setAlignment(Qt::AlignCenter);
    startProcess();
}

void MainWindow::simulateOther()
{
    patient->setState(other);
    ui->currScenarioLabel->setText("Other Rhythms");
    ui->currScenarioLabel->setAlignment(Qt::AlignCenter);
    startProcess();
}

void MainWindow::initializeMainTimer(QTimer* t)
{
    connect(t, &QTimer::timeout, this, &MainWindow::updateMainTimer);
    qDebug() << "starting process " << QString::number(currStep);
    stepImages[0]->setChecked(true); //this sets the first step button to checked, so the image will change, same for all other steps
    qDebug() << "Process 1: Checking if the patient is OK";
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
            qDebug() << "Process 2: Calling emergency service";
            updatingEcg(10,1000);
            currStep++;
        break;
        case 2://this finishes the second step and start the third step
            stepImages[1]->setChecked(false);
            stepImages[2]->setChecked(true);
            qDebug() << "starting process " << QString::number(currStep);
            qDebug() << "Process 3: Wating pad placement";
            updatingEcg(10,1000);
            currStep++;
        break;
        case 3://this tries to finishes the third step (pad detection) and if finished, start the fourth step
            padDetecting();
        break;
        case 4://this shall try to finish the fourth step (heart analyzing step) and if finished, start the fifth step
            startAnaylzing();
        break;
        case 5://this shall try to finish the fifth step (cpr) and if finished, it shall determine patient's condition, to decide whether re run the heart analyzing step again or stop the process if patient is dead

            doCpr();
             //going next step at step 5 meaning cpr is done and patient is healthy, the process shall terminates
            //otherwise currStep--, which means patient is not healthy and should continue doing shock and cpr, back to the anaylzing step (step 4) if patient still needs treatment
        break;
    }
    if(!aedWaiting){
        stopProcess();//aed not waiting meaning there is no process ongoing
        qDebug() << "stop at process " << QString::number(currStep);
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
            ui->padLabel->setPixmap(QPixmap(":/electrode/electrodeAdultPadOn"));
        }
        else{
            aed.setAdultPad(false);
            aed.setChildPad(true);
            ui->padLabel->setPixmap(QPixmap(":/electrode/electrodeChildPadOn"));
        }
    }
}

void MainWindow::placePad()
{
    patient->setPad();
    if(currStep == 4){
        if(!patient->hasPad()){
            qInfo("Pad is not attached, patient is dead");
            patient->setState(dead);
        }
    }
}

void MainWindow::waitingForPad()
{
    if((patient->hasPad() && aed.isConnected()) || waitPadTime >= 5){
        waitPadTime = 0;
        stepImages[2]->setChecked(false);
        aedWaiting = false;
        if(!patient->hasPad()){
            qDebug() << "Waiting time reached 5. No action was performed. System timed out.";
            operating = false;
            waitPadTime = 0; //reset wait time
        }
    }
    else{
        waitPadTime++;
        qDebug() << "Current waiting time is " << QString::number(waitPadTime);
    }
}

void MainWindow::padDetecting()
{
    if(patient->hasPad()&&aed.isConnected()){
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

void MainWindow::stopProcess()
{
    if(currStep <= 5 && currStep > 0){ //if users turn off device when the process (5 steps) is not over, uncheck the last step before turnning off
        stepImages[currStep-1]->setChecked(false);
    }
    ui->ecgWaveGraph->graph()->setPen(QPen(Qt::red));
    mainProcessTimer->stop();
    mainProcessTimer->disconnect();
    ui->ecgWaveGraph->xAxis->setRange(0, 50);
    ui->ecgWaveGraph->graph(0)->data()->clear();
    ui->ecgWaveGraph->replot();
    ui->currScenarioLabel->clear();
    anaylzingTime = 0;
    ecgTime = 0;
    cprCount = 0;
    patient->clearHeartData();
    ui->cprCount->display(0);
    setPatientInfo("Unknown");
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
    if(patient->getState()==healthy){
        ui->ecgWaveGraph->graph()->setPen(QPen(Qt::green));
    }
    else{
        ui->ecgWaveGraph->graph()->setPen(QPen(Qt::red));
    }
    anaylzingTime++;
    qDebug() << "anaylzing, please wait, current time elapsed for anaylzing: " << QString::number(anaylzingTime*2) << "seconds";
    if(anaylzingTime < ANALYZING_TIME){
        generateHeartData();
    }
    else{
        setPatientInfo("not Unknown");
        if(aed.detectPatientState()==dead || aed.detectPatientState()==healthy){
            aedWaiting = false;
            qDebug() << "either flatline signal, healthy signal or no signal detected, stop the process";
        }
        else if(anaylzingTime >= ANALYZING_TIME && !aed.detectShockable()){
            qDebug() << "no shock needed, on to the cpr period";
            stepImages[3]->setChecked(false);
            stepImages[4]->setChecked(true);
            setCprButtons(true);
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
            setCprButtons(true);
            anaylzingTime = 0;
            currStep++;
        }
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

    if(patient->getState()==tachycardia || patient->getState()==other || patient->getState()==healthy){//make graph resembles tachycardia and other (assuming other rhythms just have low but fixed heart rate)
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

void MainWindow::doCpr()
{
    bool cprPeriod = (cprTime <= CPR_TIME);
    if(cprPeriod){
        if(previousCpr == 0){
            qInfo("DO CPR PUSHES or PUSH HARDER");
        }
        else if(previousCpr == 1){
            qInfo("PUSH HARDER");
        }
        cprTime++;
        qDebug() << "in cpr period, please do cpr, current time elapsed for cpr:" << QString::number(cprTime*2) << "seconds";
    }
    else{
        int patientDiePossibility;
        int patientHealthyPossibility;
        stepImages[4]->setChecked(false);
        if(cprCount >= 4){
            patientDiePossibility = rand()%5+1;
            patientHealthyPossibility = rand()%3+1;
        }
        else{
            patientDiePossibility = rand()%3+1;
            patientHealthyPossibility = rand()%6+1;
        }
        if(patientDiePossibility <= 1){
            patient->setState(dead);
            qInfo("Patient is dead");
        }
        else if(patientHealthyPossibility <= 1){
            patient->setState(healthy);
            qInfo("Patient is healthy");
        }
        setCprButtons(false);
        cprCount = 0;
        cprTime = 0;
        currStep--;
        stepImages[3]->setChecked(true);
        ui->cprCount->display(cprCount);
        patient->clearHeartData();
        setPatientInfo("Unknown");
    }
}

void MainWindow::setCprButtons(bool state)
{
    ui->cprNoneDetectablePush->setEnabled(state);
    ui->cprPushHarder->setEnabled(state);
    ui->cprGoodPush->setEnabled(state);
}

void MainWindow::cprPush()
{
    for(int i=0; i<3; ++i){
        if(cprButtons[i]->isChecked()){
            if(previousCpr == 1 && i == 2){
                qInfo("GOOD PUSH, PLEASE KEEP IT UP");
            }
            previousCpr = i;
            cprButtons[i]->setChecked(false);
            if(i != 0){
                if(ui->cprBar->value() > 0){
                    cprBarDropTimer->stop();
                    cprBarDropTimer->disconnect();
                }
                ui->cprBar->setValue(i*40);
                cprBarDropTimer = new QTimer(this);
                connect(cprBarDropTimer, &QTimer::timeout, this, &MainWindow::cprBarDrop);
                cprBarDropTimer->start(250);
                if(i == 2)
                    cprCount++;
                ui->cprCount->display(cprCount);
            }
            break;
        }
    }
}

void MainWindow::cprBarDrop()
{
    if(ui->cprBar->value() >= 10)
        ui->cprBar->setValue(ui->cprBar->value()/6*5);
    else{
        ui->cprBar->setValue(0);
        previousCpr = 0;
        cprBarDropTimer->stop();
        cprBarDropTimer->disconnect();
    }
}

void MainWindow::setPatientInfo(QString state)
{
    if(state != "Unknown"){
        ui->patientInfoHeartRate->setText(QString::number(patient->getHeartRate()));
        ui->patientInfoCondition->setText(patient->getCurrState());
    }
    else{
        ui->patientInfoHeartRate->setText(state);
        ui->patientInfoCondition->setText(state);
    }
}
