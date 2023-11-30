#include "mainwindow.h"
#include "ui_mainwindow.h"

#define ANALYZING_TIME 4 //the end time for analyzing
#define CPR_TIME 10 //the end time for cpr
#define WAIT_PAD_TIME 5 //the end time for waiting pad
#define MAIN_PROCESS_TIME_INTERVAL 2000 //the time interval for main process timer, 1000 = 1s
#define ENOUGH_CPR_COUNT 4 //the enough number of cpr performed
#define CPR_BAR_DROP_RATE 250 //cpr bar dropping rate

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //create a patient and add it to aed, also set age to adult to patient panel by default
    patient = new Patient();
    aed.newPatient(patient);
    ui->patientInfoAge->setText("Adult");
    simulatedState = dead;

    //set up ecg wave graph, which is a QCustomPlot type graph, learn more at https://www.youtube.com/watch?v=peoQJhzlLi8
    ui->ecgWaveGraph->setBackground(QColor(0,0,0));
    ui->ecgWaveGraph->addGraph();
    ui->ecgWaveGraph->graph()->setPen(QPen(Qt::red));
    ui->ecgWaveGraph->xAxis->setRange(0,50);
    ui->ecgWaveGraph->yAxis->setRange(0,2100);
    ui->ecgWaveGraph->xAxis->setVisible(false);
    ui->ecgWaveGraph->yAxis->setVisible(false);

    //create timer for mainProcessTimer and cprBarDropTimer
    mainProcessTimer = new QTimer(this);
    cprBarDropTimer = new QTimer(this);
    //set all counter values, such as timer count value, currStep etc. to initial value
    waitPadTime = 0;
    anaylzingTime = 0;
    cprTime = 0;
    waveGraphX = 0.0;
    aedWorking = false;
    currStep = -1;
    cprCount = 0;
    previousCpr = -1;

    //deviced initially set to off
    operating = false;
    changeDeviceState();
    //push all step buttons to the array, and set them to disable, since they shouldn't be clicked by user
    stepImages.push_back(ui->step1);
    stepImages.push_back(ui->step2);
    stepImages.push_back(ui->step3);
    stepImages.push_back(ui->step4);
    stepImages.push_back(ui->step5);
    for(int i=0; i<5; ++i){
        stepImages[i]->setDisabled(true);
    }
    //push cpr buttons to cprButtons array
    cprButtons.push_back(ui->cprNoneDetectablePush);
    cprButtons.push_back(ui->cprPushHarder);
    cprButtons.push_back(ui->cprGoodPush);
    //set shockButton to disabled at start
    ui->shockButton->setEnabled(false);

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
    connect(ui->patientTouchedCheckBox, &QCheckBox::clicked, this, &MainWindow::setPatientTouched);
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
    //enable or disable simulateButtons and cpr buttons, if it is not in cpr period or device is not turned on, cpr buttons should be disabled
    bool cprEnabled = operating&&(currStep == 5);
    setSimulateButtons(operating);
    setCprButtons(cprEnabled);

    //set ui elements visability, also the selfcheck light label state
    if(operating)
        ui->selfTestLight->setChecked(aed.selfCheck());
    ui->statusBarQFrame->setVisible(operating);
    ui->cprBar->setVisible(operating);
    ui->ecgWaveGraph->setVisible(operating);
    ui->blackScreen->setVisible(!operating);
    ui->padLabel->setVisible(operating);
    ui->selfTestLight->setVisible(operating);
}

void MainWindow::togglePowerButton(bool checked)
{
    //if power button is on, turn on device if battery is above 0.
    if(checked){
        if(aed.getBattery()>0.0){
            operating = true;
            changeDeviceState();
            qDebug() << "turning on ";
        }
    }
    //if aed is turned off while it is still in process, set aedWorking to false and stop all processes
    else{
        operating = false;
        changeDeviceState();
        if(aedWorking){
            aedWorking = operating;
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
    aed.setConnected(connection);
    //if user connect electrode, they can no longer change pad, and aed will record which pad is connected
    if(connection){
        ui->padSelectionComboBox->setEnabled(false);
        ui->electrodeLabel->setText("Electrode connected");
        if(ui->padPlacedCheckBox->isChecked()){
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
    }
    //if user disconnect electrode, they can change pad
    else{
        ui->padSelectionComboBox->setEnabled(true);
        aedWorking = false;
        ui->electrodeLabel->setText("Electrode not connected");
        ui->padLabel->setPixmap(QPixmap(":/electrode/electrodePadOff"));
    }
    if(!aedWorking && operating)//only updates self test light when aed is not in process and aed is on
        ui->selfTestLight->setChecked(aed.selfCheck());
}

void MainWindow::simulateFib()
{
    simulatedState = fibrillation;
    patient->setState(fibrillation);

    ui->currScenarioLabel->setText("Ventricular Fibrillation");
    ui->currScenarioLabel->setAlignment(Qt::AlignCenter);
    startProcess();
}

void MainWindow::simulateTach()
{
    simulatedState = tachycardia;
    patient->setState(tachycardia);

    ui->currScenarioLabel->setText("Ventricular Tachycardia");
    ui->currScenarioLabel->setAlignment(Qt::AlignCenter);
    startProcess();
}

void MainWindow::simulateDead()
{
    simulatedState = dead;
    patient->setState(dead);

    ui->currScenarioLabel->setText("Faltlined Patient");
    ui->currScenarioLabel->setAlignment(Qt::AlignCenter);
    startProcess();
}

void MainWindow::simulateOther()
{
    simulatedState = other;
    patient->setState(other);

    ui->currScenarioLabel->setText("Other Rhythms");
    ui->currScenarioLabel->setAlignment(Qt::AlignCenter);
    startProcess();
}

void MainWindow::initializeMainTimer()
{
    connect(mainProcessTimer, &QTimer::timeout, this, &MainWindow::updateMainTimer);
    qDebug() << "starting process " << QString::number(currStep);
    stepImages[0]->setChecked(true); //this sets the first step button to checked, so the image will change, same for all other steps
    qDebug() << "Process 1: Checking if the patient is OK";
    updatingEcg(1,1000);//start a flatline ecg wave at the first step
    currStep++;
    mainProcessTimer->start(MAIN_PROCESS_TIME_INTERVAL);
}

void MainWindow::updateMainTimer()
{

    consumingBattery(1);

    if(aed.getBattery()<=0.0){
        aedWorking = false;
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
        case 2://this finishes the second step and start the third (pad detection) step
            stepImages[1]->setChecked(false);
            stepImages[2]->setChecked(true);
            qDebug() << "starting process " << QString::number(currStep);
            qDebug() << "Process 3: Wating pad placement";
            updatingEcg(10,1000);
            currStep++;
        break;
        case 3://this tries to finish the third step (pad detection) and if finished, start the fourth step
            padDetecting();
        break;
        case 4://this shall try to finish the fourth step (heart analyzing step) and if finished, start the fifth step
            startAnaylzing();
        break;
        case 5://this shall try to finish the fifth step (cpr) and if finished, it shall determine patient's condition, to decide whether re run the heart analyzing step again or stop the process if patient is dead
            doCpr();
            if(currCpr == 0){
                ui->cprPrompt->setText("DO CPR PUSHES or PUSH HARDER");
            }
            //if it is an adult pad, pushing 1 inch is not enough
            else if(currCpr == 1 && aed.hasAdultPad()){
                ui->cprPrompt->setText("PUSH HARDER");
            }
            if(previousCpr == 1 && currCpr == 2 && aed.hasAdultPad()){
                ui->cprPrompt->setText("GOOD PUSH, PLEASE KEEP IT UP");
            }
             //going next step at step 5 meaning cpr is done and patient is healthy, the process shall terminates
            //otherwise currStep--, which means patient is not healthy and should continue doing shock and cpr, back to the anaylzing step (step 4) if patient still needs treatment
        break;
    }
    //whenever aedWorking is set to false, stop the process
    if(!aedWorking){
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

void MainWindow::placePad(bool placed)
{
    //if checked place pad checkbox, check which pad is currently selected in the comboBox, and set that pad to true in aed
    if(placed){
        if(ui->padSelectionComboBox->currentText()=="Adult Pad"){
            aed.setAdultPad(true);
            aed.setChildPad(false);
        }
        else{
            aed.setAdultPad(false);
            aed.setChildPad(true);
        }
    }
    //otherwise if place pad checkbox is unchecked, set all pad in aed to false
    else{
        aed.setAdultPad(false);
        aed.setChildPad(false);
    }
    //if user detach pad during analyzing period, aed assumes patient is dead
    if(currStep == 4){
        if(!aed.detectPad()){
            qInfo("Pad is not attached, patient is dead");
            aed.setDetectedState(dead);
        }
    }
}

void MainWindow::waitingForPad()
{
    //whenever aed has a pad detected, or aed does not detect any pad after WAIT_PAD_TIME elpased, aed should turn off
    if(aed.detectPad() || waitPadTime >= WAIT_PAD_TIME){
        waitPadTime = 0;
        stepImages[2]->setChecked(false);//set step 3 (detect pad) image to gray light
        aedWorking = false;
        if(!aed.detectPad()){
            qDebug() << "Waiting time reached. No action was performed. System timed out.";
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
    //whenever pad is detected at step 3, on to the next step by adding 1 to currStep
    if(aed.detectPad()){
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
    //if users turn off device when the process (5 steps) is not over, set the last step image to gray light
    if(currStep <= 5 && currStep > 0){
        stepImages[currStep-1]->setChecked(false);
    }

    //make sure ecg waveform is set to default color (red)
    ui->ecgWaveGraph->graph()->setPen(QPen(Qt::red));

    //make sure mainProcessTimer is stopped and disconnected
    mainProcessTimer->stop();
    mainProcessTimer->disconnect();

    //make sure ecg graph is cleared
    ui->ecgWaveGraph->xAxis->setRange(0, 50);
    ui->ecgWaveGraph->graph(0)->data()->clear();
    ui->ecgWaveGraph->replot();
    ui->currScenarioLabel->clear();

    anaylzingTime = 0;
    waveGraphX = 0;
    cprCount = 0;
    aed.clearHeartData();
    ui->cprCount->display(0);
}

void MainWindow::startProcess()
{
    setPatientInfo("Unknown");
    //if aed passed self check, start the process by initializing mainProcessTimer, also disable simulate scenario buttons, pas selecting buttons and change age button
    if(aed.selfCheck()){
        if(ui->patientTouchedCheckBox->isChecked())
            aed.setDetectedState(healthy);
        else
            aed.setDetectedState(simulatedState);
        currStep = 0;
        aedWorking = true;
        initializeMainTimer();
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
    //ecg waveform should be green if patient is healthy, otherwise red
    if(aed.getDetectedState()==healthy){
        ui->ecgWaveGraph->graph()->setPen(QPen(Qt::green));
    }
    else{
        ui->ecgWaveGraph->graph()->setPen(QPen(Qt::red));
    }

    anaylzingTime++;
    qDebug() << "anaylzing, please wait, current time elapsed for anaylzing: " << QString::number(anaylzingTime*2) << "seconds";

    if(anaylzingTime < ANALYZING_TIME){
        //generate and draw ecg data every MAIN_PROCESS_TIME_INTERVAL/1000 seconds
        generateHeartData();
    }
    else{
        //update patient panel
        setPatientInfo("not Unknown");

        //if patient is either dead or healthy, stop the process
        if(aed.getDetectedState()==dead || aed.getDetectedState()==healthy){
            aedWorking = false;
            qDebug() << "either flatline signal, healthy signal or no signal detected, stop the process";
        }
        //else if patient does not have shockable rhythm, on to the cpr period
        else if(!aed.detectShockable()){
            qDebug() << "no shock needed, on to the cpr period";
            stepImages[3]->setChecked(false);
            stepImages[4]->setChecked(true);
            setCprButtons(true);
            anaylzingTime = 0;
            currStep++;
        }
        //else if patient has shockable rhythm, enable shock button and wait for user clicking it
        else if(!ui->shockButton->isChecked()){
            ui->shockButton->setEnabled(true);
            generateHeartData();
            qDebug() << "waiting for delivering shock";
        }
        //else if user has clicked the shock button, on to the cpr period
        else{
            //when shock button is clicked, drain an amount of battery base on the pad type
            double newRemainingBattery = (aed.hasAdultPad()) ? qMax(aed.getBattery() - 10, 0.0) : qMax(aed.getBattery() - 5, 0.0);
            changeBatteryLeft(newRemainingBattery);

            //then disable shockButton
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
    waveGraphX += x;
    ui->ecgWaveGraph->graph(0)->addData(waveGraphX,y);
    if(waveGraphX > 50){
        ui->ecgWaveGraph->xAxis->setRange(waveGraphX-50, waveGraphX+10);
    }
    ui->ecgWaveGraph->replot();
}

void MainWindow::generateHeartData()
{
    aed.setHeartData();

    //make graph resembles the specific heart disease (assuming other rhythms just have low but fixed heart rate)
    if(aed.getDetectedState()==tachycardia || aed.getDetectedState()==other || aed.getDetectedState()==healthy){
        updatingEcg(2,aed.getAmp()+1000);
        updatingEcg(2,1000-aed.getAmp());
        updatingEcg(2,1000);
        updatingEcg(220/aed.getHeartRate(),1000);
    }
    else if(patient->getState()==fibrillation){
        double randomInvertal = (rand()%9+4)/(rand()%3+1);
        updatingEcg(randomInvertal,aed.getAmp()+1000);
        updatingEcg(randomInvertal,aed.getAmp());
    }
    else{
        //flatline for dead patient
        updatingEcg(10,1000);
    }
}

void MainWindow::doCpr()
{
    if(cprTime <= CPR_TIME){
        cprTime++;
        qDebug() << "in cpr period, please do cpr, current time elapsed for cpr:" << QString::number(cprTime*2) << "seconds";
    }
    else{
        int patientDiePossibility;
        int patientHealthyPossibility;
        stepImages[4]->setChecked(false);

        //if patient receiving enough cpr and pad is placed correctly, they might get healthy or might die
        if(cprCount >= ENOUGH_CPR_COUNT && aed.hasAdultPad() == patient->notChild()){
            patientDiePossibility = rand()%5+1;
            patientHealthyPossibility = rand()%3+1;
        }
        //if patient not receiving enough cpr or patient has a wrong pad placed, they never get healthy
        else{
            patientDiePossibility = rand()%3+1;
            patientHealthyPossibility = 2;
        }
        if(patientDiePossibility <= 1){
            patient->setState(dead);
            aed.setDetectedState(dead);
            qInfo("Patient is dead");
        }
        else if(patientHealthyPossibility <= 1){
            patient->setState(healthy);
            aed.setDetectedState(healthy);
            qInfo("Patient is healthy");
        }
        //back to analyzing step to determine if cpr period is finished, since after doing cpr, heart data should be reanalyzed
        setCprButtons(false);
        cprCount = 0;
        cprTime = 0;
        currStep--;
        stepImages[3]->setChecked(true);
        ui->cprCount->display(cprCount);
        aed.clearHeartData();
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
    ui->cprPrompt->clear();
    for(int i=0; i<3; ++i){
        if(cprButtons[i]->isChecked()){
            //if the pad is for adult, and previous push is 1 inch, while current push is 2 inches, prompts a good push message
            previousCpr = currCpr;
            currCpr = i;
            cprButtons[i]->setChecked(false);

            //if push is above or equals to 1 inch
            if(i != 0){
                //make sure cprBarDropTimer is stopped and disconnected
                if(ui->cprBar->value() > 0){
                    cprBarDropTimer->stop();
                    cprBarDropTimer->disconnect();
                }
                ui->cprBar->setValue(i*40);
                connect(cprBarDropTimer, &QTimer::timeout, this, &MainWindow::cprBarDrop);
                cprBarDropTimer->start(CPR_BAR_DROP_RATE);

                //if it is a child pad, pushing 1 or 2 inches should be a good push, otherwise only 2 inches is a good push
                if(!aed.hasAdultPad() || (i == 2 && aed.hasAdultPad()))
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
        ui->patientInfoHeartRate->setText(QString::number(aed.getHeartRate()));
        ui->patientInfoCondition->setText(patient->getStateString());
        ui->detectedInfoCondition->setText(aed.getDetectedStateString());
    }
    else{
        ui->patientInfoHeartRate->setText(state);
        ui->patientInfoCondition->setText(state);
        ui->detectedInfoCondition->setText(state);
    }
}

void MainWindow::setPatientTouched(bool touched)
{
    //assuming touching patient makes aed detect a healthy rhythm
    if(touched){
        aed.setDetectedState(healthy);
    }
    else{
        aed.setDetectedState(simulatedState);
    }
}
