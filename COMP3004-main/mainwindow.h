#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include "patient.h"
#include "aed.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Aed aed;
    Patient* patient;

    QTimer* mainProcessTimer;
    int currStep;
    int anaylzingTime;
    int cprTime;
    bool nextStep;
    bool operating;
    bool coonnected;
    QVector<QPushButton*> stepImages;

    void changeDeviceState();
    void consumingBattery(double);
    void initializeMainTimer(QTimer*);
    void initializeAedTimer(QTimer*);
private slots:
    void createPatient();
    void fillBattery();
    void togglePowerButton(bool);
    void changeBatteryLeft(double);
    void connectElectrode(bool);
    void simulateFib();
    void simulateTach();
    void simulateDead();
    void simulateOther();
    void updateMainTimer();
};
#endif // MAINWINDOW_H
