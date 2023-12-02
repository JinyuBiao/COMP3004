#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    dataProcessor = new DataProcessor();
    ui->setupUi(this);
    //create a patient and add it to aed,
    //also set age to adult to patient panel by default
    patient = new Patient();
    ui->patientInfoAge->setText("Adult");
    simulatedState = dead;

    //set up ecg wave graph, which is a QCustomPlot type graph,
    //learn more at https://www.youtube.com/watch?v=peoQJhzlLi8
    initializeECGWaveGraph();

    //create timer for mainProcessTimer and cprBarDropTimer
    mainProcessTimer = new QTimer(this);
    cprBarDropTimer = new QTimer(this);

    //set all counter values,
    //such as timer count value, currStep etc. to initial value
    initializeCounterValues();

    //deviced initially set to off
    operating = false;
    changeDeviceState();

    //push all step buttons to the array
    initializeButtons();

    //set shockButton to disabled at start
    ui->shockButton->setEnabled(false);

    //connect buttons
    connectButtons();

}
//[helper methods]
void MainWindow::initializeCounterValues(){
    //set all counter values,
    //such as timer count value, currStep etc. to initial value
    waitPadTime = DEFAULT_WAIT_PAD_TIME;
    anaylzingTime = DEFAULT_ANALYZING_TIME;
    cprTime = DEFAULT_CPR_TIME;
    waveGraphX = DEFAULT_WAVE_GRAPH_X;
    aedWorking = DEFAULT_AED_WORKING;
    currStep = DEFAULT_CURRSTEP;
    cprCount = DEFAULT_CPRCOUNT;
    previousCpr = DEFAULT_PREVIOUSCPR;
}

void MainWindow::initializeButtons(){

    //push all step buttons to the array,
    //and set them to disable, since they shouldn't be clicked by user
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

}

void MainWindow::initializeECGWaveGraph(){

    //set up ecg wave graph, which is a QCustomPlot type graph,
    //learn more at https://www.youtube.com/watch?v=peoQJhzlLi8
    ui->ecgWaveGraph->setBackground(QColor(0,0,0));
    ui->ecgWaveGraph->addGraph();
    ui->ecgWaveGraph->graph()->setPen(QPen(Qt::red));
    ui->ecgWaveGraph->xAxis->setRange(0,50);
    ui->ecgWaveGraph->yAxis->setRange(0,2100);
    ui->ecgWaveGraph->xAxis->setVisible(false);
    ui->ecgWaveGraph->yAxis->setVisible(false);

}

void MainWindow::connectButtons(){

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
    connect(ui->shockButton, &QCheckBox::clicked, this, &MainWindow::givingShock);
}

void MainWindow::disconnectElectrode(){
    ui->padSelectionComboBox->setEnabled(true);
    aedWorking = false;
    ui->electrodeLabel->setText("Electrode not connected");
    ui->padLabel->setPixmap(QPixmap(":/electrode/electrodePadOff"));
}

void MainWindow::connectedToAdultPad(){
    dataProcessor->setAdultPad(true);
    dataProcessor->setChildPad(false);
    ui->padLabel->setPixmap(QPixmap(":/electrode/electrodeAdultPadOn"));
}

void MainWindow::connectedToChildPad(){
    dataProcessor->setAdultPad(false);
    dataProcessor->setChildPad(true);
    ui->padLabel->setPixmap(QPixmap(":/electrode/electrodeChildPadOn"));
}

void MainWindow::cprBarDropHelper(int i){
    if(dataProcessor->detectPad()){
    //if push is above or equals to 1 inch
        if(i != 0){
            ui->cprPrompt->clear();
            cprPrompt();
            //make sure cprBarDropTimer is stopped
            //and disconnected
            if(ui->cprBar->value() > 0){
                cprBarDropTimer->stop();
                cprBarDropTimer->disconnect();
            }
            ui->cprBar->setValue(i*CPR_BAR_MULTIPLIAR);
            connect(cprBarDropTimer,
                    &QTimer::timeout,
                    this, &MainWindow::cprBarDrop);
            cprBarDropTimer->start(CPR_BAR_DROP_RATE);

            //if it is a child pad, pushing 1 or 2 inches should be a good push, otherwise only 2 inches is a good push
            if(dataProcessor->hasChildPad() ||
                    (currCpr == CPR_COMPRESSION_LEVEL_C_INCH
                     && dataProcessor->hasAdultPad()))
               cprCount++;
            ui->cprCount->display(cprCount);
        }
    }

}

void MainWindow::determinePatientSurvival(){
    int patientDiePossibility;
    int patientHealthyPossibility;
    stepImages[4]->setChecked(false);

    //if patient receiving enough cpr and pad is placed correctly,
    //they might get healthy or might die
    if(cprCount >= ENOUGH_CPR_COUNT
            && dataProcessor->hasAdultPad()
            == patient->notChild()){
        patientDiePossibility = rand()%
                CPR_PATIENT_DIE_POSSIBILITY_FACTOR+1;
        patientHealthyPossibility = rand()%
                CPR_PATIENT_HEALTHY_POSSIBILITY_FACTOR+1;
    }
    //if patient not receiving enough cpr
    //or patient has a wrong pad placed, they never get healthy
    else{
        patientDiePossibility = rand()%
                CPR_NOT_ENOUGH_PATIENT_HEALTHY_POSSIBILITY_FACTOR+1;
        //impossible to survive
        patientHealthyPossibility = CPR_IMPOSSIBLE;
    }


    if(patientDiePossibility <= 1){
        patient->setState(dead);
        dataProcessor->setDetectedState(dead);
        qInfo("Patient is dead");
    }
    else if(patientHealthyPossibility <= 1){
        patient->setState(healthy);
        dataProcessor->setDetectedState(healthy);
        qInfo("Patient is healthy");
    }
    //back to analyzing step to determine if cpr period is finished, since after doing cpr, heart data should be reanalyzed
    setCprButtons(false);
    cprCount = 0;
    cprTime = 0;
    currStep--;
    stepImages[3]->setChecked(true);
    dataProcessor->clearHeartData();
    setPatientInfo("Unknown");
}

//[heavy lifting]
MainWindow::~MainWindow()
{
    delete dataProcessor;
    delete ui;
    delete mainProcessTimer;
    delete cprBarDropTimer;
    delete patient;
}

//button presses
void MainWindow::changeDeviceState()
{
    //enable or disable simulateButtons and cpr buttons,
    setSimulateButtons(operating);
    setCprButtons(false);

    //set ui elements visability, also the selfcheck light label state
    if(operating)
        ui->selfTestLight->setChecked(dataProcessor->selfCheck());
    ui->statusBarQFrame->setVisible(operating);
    ui->cprBar->setVisible(operating);
    ui->ecgWaveGraph->setVisible(operating);
    ui->blackScreen->setVisible(!operating);
    ui->padLabel->setVisible(operating);
    ui->selfTestLight->setVisible(operating);
    ui->cprPrompt->clear();
}

void MainWindow::togglePowerButton(bool checked)
{
    //if power button is on, turn on device if battery is above 0.
    if(checked){
        if(dataProcessor->getBattery()>0.0){
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
            stopProcess();
        }
        qDebug() << "turning off ";
    }
    ui->powerOnButton->setChecked(operating);
}

//battery functions
void MainWindow::changeBatteryLeft(double batteryLeft)
{
    if(batteryLeft >= 0.0 && batteryLeft <= 100.0){
        if(batteryLeft <= 0 && operating){
            togglePowerButton(false);
        }
        dataProcessor->setBattery(batteryLeft);
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
    double currBattery = dataProcessor->getBattery();
    currBattery = qMax((currBattery - consumption),0.0);
    changeBatteryLeft(currBattery);
}

//electrode
void MainWindow::connectElectrode(bool connection)
{
    dataProcessor->setConnected(connection);
    //if user connect electrode,
    //they can no longer change pad,
    //and aed will record which pad is connected
    if(connection){
        ui->padSelectionComboBox->setEnabled(false);
        ui->electrodeLabel->setText("Electrode connected");
        if(ui->padPlacedCheckBox->isChecked()){
            if(ui->padSelectionComboBox->currentText()=="AdultPad"){
                connectedToAdultPad();
            }
            else{
                connectedToChildPad();
            }
        }
    }
    //if user disconnect electrode, they can change pad
    else{
        disconnectElectrode();
    }
    if(!aedWorking && operating)
        //only updates self test light when aed is not in process and aed is on
        ui->selfTestLight->setChecked(dataProcessor->selfCheck());
}

//simulate different cases
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

//timer operations
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

    if(dataProcessor->getBattery()<=0.0){
        aedWorking = false;
    }
    switch(currStep){
        case 1:
        //this finishes the first step
        //(set first step button to unchecked) and start the second step
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
            updatingEcg(10,1000);
            padDetecting();
        break;
        case 4://this shall try to finish the fourth step (heart analyzing step) and if finished, start the fifth step
            startAnaylzing();
            //separate class
        break;
        case 5://this shall try to finish the fifth step (cpr) and if finished, it shall determine patient's condition, to decide whether re run the heart analyzing step again or stop the process if patient is dead
            ui->cprCount->display(cprCount);
            if(currCpr == 0){
                ui->cprPrompt->setText("DO CPR PUSHES or PUSH HARDER");
            }
            else{
                cprPrompt();
            }
            //if it is an adult pad, pushing 1 inch is not enough
            currCpr = 0;
            doCpr();
            //separate class
        break;
    }
    //whenever aedWorking is set to false, stop the process
    if(!aedWorking){
        qDebug() << "stop at process " << QString::number(currStep);
        stopProcess();//aed not waiting meaning there is no process o ngoing
        if(!operating)
            togglePowerButton(false);
        else if(!dataProcessor->isConnected())
            qInfo("electrode unplugged during the process, stop working!");
        //setSimulateButtons(operating); //whenever aed is not in a treatment process, all scenario simulation buttons should be enabled
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
    if(dataProcessor->isConnected()){
        if(index == 0){
            dataProcessor->setAdultPad(true);
            dataProcessor->setChildPad(false);
            ui->padLabel->setPixmap(QPixmap(":/electrode/electrodeAdultPadOn"));
        }
        else{
            dataProcessor->setAdultPad(false);
            dataProcessor->setChildPad(true);
            ui->padLabel->setPixmap(QPixmap(":/electrode/electrodeChildPadOn"));
        }
    }
}

void MainWindow::placePad(bool placed)
{
    //if checked place pad checkbox,
    //check which pad is currently selected in the comboBox,
    //and set that pad to true in aed
    if(placed){
        if(ui->padSelectionComboBox->currentText()=="Adult Pad"){
            dataProcessor->setAdultPad(true);
            dataProcessor->setChildPad(false);
        }
        else{
            dataProcessor->setAdultPad(false);
            dataProcessor->setChildPad(true);
        }
    }
    //otherwise if place pad checkbox is unchecked, set all pad in aed to false
    else{
        dataProcessor->setAdultPad(false);
        dataProcessor->setChildPad(false);
    }
    //if user detach pad during analyzing period, aed assumes patient is dead
    if(currStep == 4){
        if(!dataProcessor->detectPad()){
            qInfo("Pad is not attached, patient is dead");
            dataProcessor->setDetectedState(dead);
        }
        else{
            dataProcessor->setDetectedState(simulatedState);
        }
    }
}

void MainWindow::stopProcess()
{
    //if users turn off device when the process (5 steps) is not over,
    //set the last step image to gray light
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
    ui->cprPrompt->clear();

    aedWorking = false;
    anaylzingTime = 0;
    waveGraphX = 0;
    cprCount = 0;
    ui->cprCount->display(cprCount);
    dataProcessor->clearHeartData();
    setCprButtons(false);
    ui->shockButton->setEnabled(false);
}

void MainWindow::startProcess()
{
    ui->cprPrompt->clear();
    setPatientInfo("Unknown");
    //if aed passed self check,
    //start the process by initializing mainProcessTimer,
    //also disable simulate scenario buttons,
    //pass selecting buttons and change age button
    if(dataProcessor->selfCheck()){
        if(ui->patientTouchedCheckBox->isChecked())
            dataProcessor->setDetectedState(healthy);
        else
            dataProcessor->setDetectedState(simulatedState);
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
    //clear past prompt
    ui->cprPrompt->clear();


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
        if(dataProcessor->getDetectedState()==dead || dataProcessor->getDetectedState()==healthy){
            aedWorking = false;
            generateHeartData();
            qDebug() << "either flatline signal, healthy signal or no signal detected, stop the process";
        }
        //else if patient does not have shockable rhythm, on to the cpr period
        else if(!dataProcessor->detectShockable()){
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
            givingShock();
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

void MainWindow::givingShock()
{
    ui->shockButton->setEnabled(false);
    double newRemainingBattery = (dataProcessor->hasAdultPad()) ? qMax(dataProcessor->getBattery() - 10, 0.0) : qMax(dataProcessor->getBattery() - 5, 0.0);
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

void MainWindow::generateHeartData()
{
    dataProcessor->setHeartData();
    if(dataProcessor->getDetectedState()==healthy){
        ui->ecgWaveGraph->graph()->setPen(QPen(Qt::green));
    }
    else{
        ui->ecgWaveGraph->graph()->setPen(QPen(Qt::red));
    }
    //make graph resembles the specific heart disease
    //(assuming other rhythms just have low but fixed heart rate)
    if(dataProcessor->getDetectedState()==tachycardia || dataProcessor->getDetectedState()==other){
        updatingEcg(2,dataProcessor->getAmp()+1000);
        updatingEcg(2,1000-dataProcessor->getAmp());
        updatingEcg(2,1000);
        updatingEcg(220/dataProcessor->getHeartRate(),1000);
    }
    else if(dataProcessor->getDetectedState()==healthy){
        updatingEcg(3,dataProcessor->getAmp()+1000);
        updatingEcg(3,1000-dataProcessor->getAmp());
        updatingEcg(4,1000);
        updatingEcg(220/dataProcessor->getHeartRate(),1000);
    }
    else if(dataProcessor->getDetectedState()==fibrillation){
        double randomInvertal = (rand()%9+4)/(rand()%3+1);
        updatingEcg(randomInvertal,dataProcessor->getAmp()+1000);
        updatingEcg(randomInvertal,dataProcessor->getAmp());
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
        qDebug() << "in cpr period, please do cpr, current time elapsed for cpr:"
                 << QString::number(cprTime*2) << "seconds";
    }
    else{
        determinePatientSurvival();
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
    currCpr = 0;
    for(int i=0; i<3; ++i){
        if(cprButtons[i]->isChecked()){
            currCpr = i;
            cprButtons[i]->setChecked(false);
            if(dataProcessor->detectPad()){
            //if push is above or equals to 1 inch
                cprBarDropHelper(i);
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
        cprBarDropTimer->stop();
        cprBarDropTimer->disconnect();
    }
}

void MainWindow::setPatientInfo(QString state)
{
    if(state != "Unknown"){
        ui->patientInfoHeartRate->setText(QString::number(dataProcessor->getHeartRate()));
        ui->patientInfoCondition->setText(patient->getStateString());
        ui->detectedInfoCondition->setText(dataProcessor->getDetectedStateString());
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
        dataProcessor->setDetectedState(healthy);
    }
    else{
        dataProcessor->setDetectedState(simulatedState);
    }
}

void MainWindow::cprPrompt()
{
    if(currCpr != 0){
        if(currCpr == CPR_COMPRESSION_LEVEL_B_INCH
                && dataProcessor->hasAdultPad()){
            ui->cprPrompt->setText("PUSH HARDER");
        }
        //if the pad is for adult,
        //and previous push is 1 inch,
        //while current push is 2 inches, prompts a good push message
        else if((previousCpr == CPR_COMPRESSION_LEVEL_B_INCH
                 && currCpr == CPR_COMPRESSION_LEVEL_C_INCH
                 && dataProcessor->hasAdultPad())
                || dataProcessor->hasChildPad()){
            ui->cprPrompt->setText("GOOD PUSH, PLEASE KEEP IT UP");
        }
        previousCpr = currCpr;
    }
}

void MainWindow::waitingForPad(){
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

void MainWindow::padDetecting(){
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
