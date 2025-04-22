#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "jfile.h"
#include <QDebug>
#include <QTextEdit>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QDateTime>
#include <QComboBox>
#include <QMessageBox>
#include <vector>

using namespace std;

mainwindow::mainwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mainwindow)
{
    ui->setupUi(this);

    chartView = new QChartView(new QChart(), ui->chartContainer);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(ui->chartContainer->size());

    connect(ui->stationComboBox, QOverload<int>::of(&QComboBox::activated), this, [=]() {
        checkAndDrawChart();
    });

    connect(ui->parameterComboBox, QOverload<int>::of(&QComboBox::activated), this, [=]() {
        checkAndDrawChart();
    });
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

void mainwindow::on_searchBtn_clicked()
{
    try{
        string city = mainwindow::getTextBox();
        myFile_ = new Jfile(city);
        populateStationComboBox();
        if(myFile_ -> HTTPError_){
            QMessageBox::warning(this, "Błąd", QString::fromStdString("Wystąpił błąd przy zapytaniu HTTP. Wykorzystamy dane przechowywane lokalnie.\n" + myFile_ -> errorMessage_));
        }
    }catch(const exception& e){
        QMessageBox::critical(this, "Błąd", QString::fromStdString(e.what()));
    }

}

void mainwindow::populateStationComboBox(){
    // Clear any previous data
    stationMap.clear();
    ui -> stationComboBox -> clear();
    ui -> stationComboBox -> addItem("Wybierz stację pomiarową");

    try{
        for(const auto& station : myFile_ -> getStations()){
            QString name = QString::fromStdString(station);
            ui -> stationComboBox -> addItem(name);
        }
    }catch(const exception& e){
        QMessageBox::critical(this, "Błąd", QString::fromStdString(e.what()));
    }

    ui -> stationComboBox -> addItem("Średnia wszystkich stacji");
}

void mainwindow::populateParameterComboBox(const QString& station){
    ui -> parameterComboBox -> clear();
    ui -> parameterComboBox -> addItem("Wybierz parametr powietrza");
    vector<string> params;

    try{
        // Check what parameter options to display
        if(station == "Średnia wszystkich stacji"){ // If average, display all available
            params = myFile_->getParams();
        }else{ // If single station, display only station's options
            params = myFile_->getStationParams(station);
        }
    }catch(const exception& e){
        QMessageBox::critical(this, "Błąd", QString::fromStdString(e.what()));
    }

    for(const auto& param : params){
        QString name = QString::fromStdString(param);
        ui -> parameterComboBox -> addItem(name);
    }
}

void mainwindow::displayChart(const QVector<QPointF> &points, const QString &title) {
    if (points.isEmpty()) {
        QMessageBox::critical(this, "Błąd", "Brak pomiarów dla podanych parametrów");
        return;
    }

    QLineSeries *series = new QLineSeries();
    for (const QPointF &pt : points)
        series->append(pt);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("dd-MM HH:mm");
    axisX->setTitleText("Czas");

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Wartość");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chartView->setChart(chart);
}

void mainwindow::checkAndDrawChart() {
    QString selectedStation = ui->stationComboBox->currentText();
    QString selectedParam = ui->parameterComboBox->currentText();

    // Sprawdzamy czy użytkownik nie zostawił domyślnych opcji
    if (selectedStation != "Wybierz stację pomiarową" &&
        selectedStation != ""){
        //if(selectedStation != ""){
            populateParameterComboBox(selectedStation);
        //}
        if(selectedParam != "Wybierz parametr powietrza" &&
            selectedParam != ""){
            try{
                QString title;
                if(selectedStation == "Średnia wszystkich stacji"){
                    title = "Uśrednione pomiary "+ selectedParam + " w mieście " + myFile_->getCity();
                }else{
                    title = "Pomiary " + selectedParam + " ze stacji pomiarowej " + selectedStation;
                }
                displayChart(myFile_->getDataPoints(selectedStation.toStdString(), selectedParam.toStdString()), title);
            }catch(const exception& e){
                QMessageBox::critical(this, "Błąd", QString::fromStdString(e.what()));
            }

        }
    }
}
