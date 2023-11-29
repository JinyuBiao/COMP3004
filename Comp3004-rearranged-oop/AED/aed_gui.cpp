#include "aed_gui.h"
#include "ui_aed_gui.h"

AED_GUI::AED_GUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AED_GUI)
{
    ui->setupUi(this);
}

AED_GUI::~AED_GUI()
{
    delete ui;
}

