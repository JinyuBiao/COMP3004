#ifndef PATIENT_H
#define PATIENT_H

#include <QString>
#include <QtMath>

typedef enum {fibrillation, tachycardia, dead, healthy, other} StateType;

/* Purpose of Class: To store information about patient
 *
 * Data Members:
 *  bool isAdult: indicate whether patient is adult or not, it is used for generate heart rate data
 *  StateType currState: the current actual state of patient, it is used for touching patient scenario for showing that aed detects wrong heart data
 *
 * Class Functions:
 *  Getters and Setters
 *  QString getStateString(): returns currState in string
 *  void changeAge(): change the isAdult bool value
 */

class Patient
{
    public:
        Patient(bool=true,StateType=dead);
        ~Patient();
        void changeAge();
        void setState(StateType);

        bool notChild();
        StateType getState();
        QString getStateString();
    private:
        bool isAdult;
        StateType currState;
};

#endif // PATIENT_H
