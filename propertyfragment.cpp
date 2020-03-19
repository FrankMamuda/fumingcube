#include "propertyfragment.h"
#include "ui_propertyfragment.h"

PropertyFragment::PropertyFragment(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PropertyFragment)
{
    ui->setupUi(this);
}

PropertyFragment::~PropertyFragment()
{
    delete ui;
}
