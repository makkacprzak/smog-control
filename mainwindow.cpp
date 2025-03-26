#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "fetchdata.h"
#include <QDebug>
#include <QTextEdit>

using namespace std;

mainwindow::mainwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mainwindow)
{
    ui->setupUi(this);
}

mainwindow::~mainwindow()
{
    delete ui;
}

std::string mainwindow::getTextBox(){
    QString data = ui -> cityInput -> toPlainText();
    ui -> cityInput -> clear();
    return data.toStdString();
}

void mainwindow::appendInformation(std::string info){
    QString data = QString::fromStdString(info);
    ui -> displayData -> append(data);
}

void mainwindow::on_searchBtn_clicked()
{
    string city = mainwindow::getTextBox();
    appendInformation(getStationID(city));
}
