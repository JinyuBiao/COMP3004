#ifndef DEFAULTHEADER_H
#define DEFAULTHEADER_H

#include <QObject>
#include <QTimer>
using namespace std;

const int REQ_ADULT_AGE = 18;
const int REQ_MIN_CHILD_AGE = 8;

const int BATTERY_FULL_ENERGY_J = 18135;
// this is taken from a CRI 123A battery
// with 3.25Vots
// of approx. 1550mAh
// which is 5.0 Wh,
// converting from 5.0 Wh to Joules
// 5.0 Wh * 3600(J/Wh)
// = 18135J of energy.

const int ENERGY_J_1 = 120;
const int ENERGY_J_2 = 150;
const int ENERGY_J_3 = 200;

const int SEC_TO_MS = 1000;
const int DEFAULT_CHARGE_TIME_SEC = 9;
const int CPR_RATE_MIN = 60;
const int CPR_RATE_MAX = 100;

const float DEPTH_CHECK_MIN_CM = 1.9;
const float DEPTH_CHECK_MAX_CM = 8.9;

const int FIBRILLATION_ABOVE_AVG_AMPLITUDE = 100;
const int TACHYCARDIA_ADULT_ABOVE_BPM = 150;
const int TACHYCARDIA_PEDIATRIC_ABOVE_BPM = 200;

typedef enum {
    fibrillation,
    tachycardia,
    dead,
    healthy,
    other} PatientStateType;


#endif // DEFAULTHEADER_H
