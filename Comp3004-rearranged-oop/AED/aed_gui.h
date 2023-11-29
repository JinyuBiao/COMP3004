#ifndef AED_GUI_H
#define AED_GUI_H

#include <QMainWindow>
#include <QObject>

QT_BEGIN_NAMESPACE
namespace Ui { class AED_GUI; }
QT_END_NAMESPACE

class AED_GUI : public QMainWindow
{
    Q_OBJECT

public:
    AED_GUI(QWidget *parent = nullptr);
    ~AED_GUI();

private:
    Ui::AED_GUI *ui;
//private slots:
//    void togglePowerButton(bool);
//    void cprPush();
//    void cprBarDrop();
};
#endif // AED_GUI_H
