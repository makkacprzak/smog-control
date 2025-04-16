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

    connect(ui->stationComboBox, &QComboBox::currentTextChanged, this, [=]() {
        checkAndDrawChart();
    });

    connect(ui->parameterComboBox, &QComboBox::currentTextChanged, this, [=]() {
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

void mainwindow::appendInformation(std::string info){
    QString data = QString::fromStdString(info);
    ui -> displayData -> append(data);
}

void mainwindow::on_searchBtn_clicked()
{
    string city = mainwindow::getTextBox();
    myFile_ = new Jfile(city);
    populateStationComboBox(myFile_);
    populateParameterComboBox(myFile_);
}

void mainwindow::populateStationComboBox(const Jfile* data){
    // Clear any previous data
    stationMap.clear();
    ui -> stationComboBox -> clear();
    ui -> stationComboBox -> addItem("Wybierz stację pomiarową");

    vector<string> stations = data -> getStations();

    for(const auto& station : stations){
        QString name = QString::fromStdString(station);
        ui -> stationComboBox -> addItem(name);
    }

    ui -> stationComboBox -> addItem("Średnia wszystkich stacji");
}

void mainwindow::populateParameterComboBox(const Jfile* data){
    ui -> parameterComboBox -> clear();
    ui -> parameterComboBox -> addItem("Wybierz parametr powietrza");

    vector<string> params = data -> getParams();

    for(const auto& param : params){
        QString name = QString::fromStdString(param);
        ui -> parameterComboBox -> addItem(name);
    }
}



void mainwindow::displayChart(const QVector<QPointF> &points, const QString &title) {
    if (points.isEmpty()) {
        qDebug() << "Wektor jest pusty!";
    }
    for (const QPointF &point : points) {
        qDebug() << "x:" << point.x() << ", y:" << point.y();
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
        selectedStation != "" &&
        selectedParam != "Wybierz parametr powietrza" &&
        selectedParam != "")
    {
        displayChart(myFile_->getDataPoints(selectedStation.toStdString(), selectedParam.toStdString()), "test");
    }
}
