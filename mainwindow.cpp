#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "jfile.h"
#include "translate.h"
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
#include <QBarSet>
#include <QBarSeries>
#include <QBarCategoryAxis>

#define _(phrase, lang) string(Translate((phrase), (lang)))
/// @brief Macro for converting std::strings to QStrings, since the method is crazy long
#define qstr(phrase) QString::fromStdString(phrase)

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

    connect(ui->checkLineGraph, &QCheckBox::toggled, this ,[this](bool checked){
        if(checked){
            ui->checkBarGraph->setChecked(false);
        }else{
            ui->checkBarGraph->setChecked(true);
        }
    });

    connect(ui->checkBarGraph, &QCheckBox::toggled, this, [this](bool checked){
        if(checked){
            ui->checkLineGraph->setChecked(false);
        }else{
            ui->checkLineGraph->setChecked(true);
        }
    });

    lang_ = "pl";
    populateLangSelect(lang_);
    connect(ui->langSelect, QOverload<int>::of(&QComboBox::activated), this, [=]() {
        changeLang();
    });
}

mainwindow::~mainwindow()
{
    delete ui;
}

void mainwindow::populateLangSelect(const string& currentLang){
    const vector<string> langs = Translate::getLangs();
    ui->langSelect->clear();
    ui->langSelect->setPlaceholderText(qstr(_("Język", lang_)));
    for(const auto& lang : langs){
        if(lang != currentLang){
            ui->langSelect->addItem(qstr(lang));
        }
    }
}

void mainwindow::changeLang(){
    const string newLang = ui->langSelect->currentText().toStdString();
    lang_ = newLang;
    ui -> cityInput ->clear();
    ui ->cityInput->setPlaceholderText(qstr(_("Wpisz nazwę miasta", lang_)));
    ui ->stationComboBox->clear();
    ui->parameterComboBox->clear();
    ui->searchBtn->setText(qstr(_("Szukaj", lang_)));
    ui->infoLabel->setText(qstr(_("Zmiany zostaną zaaplikowane dopiero po wybraniu nowego parametru", lang_)));
    ui->checkLineGraph->setText(qstr(_("Wykres Liniowy", lang_)));
    ui->checkBarGraph->setText(qstr(_("Wykres Słupkowy", lang_)));
    populateLangSelect(lang_);
}

std::string mainwindow::getTextBox() const{
    QString data = ui -> cityInput -> toPlainText();
    ui -> cityInput -> clear();
    return data.toStdString();
}

void mainwindow::on_searchBtn_clicked()
{
    try{
        delete myFile_;
        myFile_ = nullptr;
        string city = mainwindow::getTextBox();
        myFile_ = new Jfile(city, lang_);
        populateStationComboBox();
        if(myFile_ -> HTTPError_){
            QMessageBox::warning(this, qstr(_("Uwaga", lang_)), qstr(_("Wystąpił błąd przy zapytaniu HTTP. Wykorzystamy dane przechowywane lokalnie.\n", lang_)) + qstr(myFile_ -> errorMessage_));
        }
    }catch(const exception& e){
        QMessageBox::critical(this, qstr(_("Błąd", lang_)), qstr(e.what()));
    }

}

void mainwindow::populateStationComboBox(){
    ui -> stationComboBox -> clear();
    ui -> stationComboBox -> addItem(qstr(_("Wybierz stację pomiarową", lang_)), QVariant("Wybierz stację pomiarową"));

    try{
        for(const auto& station : myFile_ -> getStations()){
            QString name = qstr(station);
            ui -> stationComboBox -> addItem(qstr(_(name.toStdString(), lang_)), QVariant(name));
        }
    }catch(const exception& e){
        QMessageBox::critical(this, qstr(_("Błąd", lang_)), qstr(e.what()));
    }

    ui -> stationComboBox -> addItem(qstr(_("Średnia wszystkich stacji", lang_)), QVariant("Średnia wszystkich stacji"));
}

void mainwindow::populateParameterComboBox(const QString& station){
    ui -> parameterComboBox -> clear();
    ui -> parameterComboBox -> addItem(qstr(_("Wybierz parametr powietrza", lang_)), QVariant("Wybierz parametr powietrza"));
    vector<string> params;

    try{
        // Check what parameter options to display
        if(station == "Średnia wszystkich stacji"){ // If average, display all available
            params = myFile_->getParams();
        }else{ // If single station, display only station's options
            params = myFile_->getStationParams(station);
        }
    }catch(const exception& e){
        QMessageBox::critical(this, "Błąd", qstr(e.what()));
    }
    for(const auto& param : params){
        QString name = qstr(param);
        ui -> parameterComboBox -> addItem(qstr(_(name.toStdString(), lang_)), QVariant(name));
    }
}

void mainwindow::displayChart(const QVector<QPointF>& points, const QString& station, const QString& param) {
    // CRUCIAL!!! Because chartView -> chart() uses a parent-child structure,
    // clearing memory reserved for a QChart object also clears any QSeries or QSet object used in it.
    // Because of some QCharts tomfoolery, since QT6 QChartView -> charts() cannot be nullptr
    // I have sort of solved that by adding if(oldChart) delete oldChart; after each chartView->setChart(chart); assignment.
    // In theory this should be memory safe. However as we know, this is C++.
    QChart *oldChart = chartView->chart();

    if (points.isEmpty()) {
        QMessageBox::critical(this, qstr(_("Błąd", lang_)), qstr(_("Brak pomiarów dla podanych parametrów", lang_)));
        return;
    }

    if(ui->checkLineGraph->isChecked()){
        QLineSeries *series = new QLineSeries();
        for (const QPointF &pt : points)
            series->append(pt);
        series -> setName(param);
        QChart *chart = new QChart();
        chart->addSeries(series);
        QString title;
        if(station == "Średnia wszystkich stacji"){
            title = qstr(_("Uśrednione pomiary ", lang_)) + qstr(_(station.toStdString(), lang_)) + qstr(_(" w mieście ", lang_)) + myFile_->getCity();
        }else{
            title = qstr(_(param.toStdString(), lang_)) + qstr(_(" w okolicy ", lang_)) + station;
        }
        chart->setTitle(title);

        QDateTimeAxis *axisX = new QDateTimeAxis;
        axisX->setFormat("dd-MM HH:mm");
        axisX->setTitleText(qstr(_("Czas", lang_)));

        QValueAxis *axisY = new QValueAxis;
        axisY->setTitleText(qstr(_("Wartość", lang_)));

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setAnimationDuration(3000);

        chartView->setChart(chart);
        if(oldChart) delete oldChart;
        return;
    }else if(ui->checkBarGraph->isChecked()){
        QPointF minPoint = points[0];
        QPointF maxPoint = points[0];
        // Find extremes
        for (const QPointF &p : points) {
            if (p.y() < minPoint.y()) minPoint = p;
            if (p.y() > maxPoint.y()) maxPoint = p;
        }
        // Convert extremes' time to QString
        QString minLabel = QDateTime::fromMSecsSinceEpoch(minPoint.x()).toString("dd.MM hh:mm");
        QString maxLabel = QDateTime::fromMSecsSinceEpoch(maxPoint.x()).toString("dd.MM hh:mm");
        // Create a set for series
        QBarSet *set = new QBarSet("Min/Max");
        *set << minPoint.y() << maxPoint.y();
        // Create a series from set
        QBarSeries *series =  new QBarSeries();
        series ->append(set);
        // Create a chart from series
        QChart *chart = new QChart();
        chart->addSeries(series);

        QString title;
        if(station == "Średnia wszystkich stacji"){
            title = qstr(_("Min/Max wartości ", lang_)) + qstr(_(param.toStdString(), lang_)) + qstr(_(" w mieście ", lang_)) + myFile_->getCity();
        }else{
            title = qstr(_("Min/Max wartości ", lang_)) + qstr(_(param.toStdString(), lang_)) + qstr(_(" w okolicy ", lang_)) + station;
        }
        chart->setTitle(title);

        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setAnimationDuration(3000);
        // Create list of labels
        QStringList categories;
        categories << minLabel << maxLabel;
        // X axis
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX -> append(categories);
        chart -> addAxis(axisX, Qt::AlignBottom);
        // Y axis
        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0, maxPoint.y() * 1.1);
        chart -> addAxis(axisY, Qt::AlignLeft);
        series -> attachAxis(axisY);

        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setChart(chart);
        if(oldChart) delete oldChart;
        return;
    }else{
        QMessageBox::critical(this, qstr(_("Błąd", lang_)), qstr(_("Nie wybrano opcji grafu",lang_)));
        return;
    }
}

void mainwindow::checkAndDrawChart() {
    QString selectedStation = ui->stationComboBox->currentData().toString();
    QString selectedParam = ui->parameterComboBox->currentData().toString();

    // Sprawdzamy czy użytkownik nie zostawił domyślnych opcji
    if (selectedStation != "Wybierz stację pomiarową" &&
        selectedStation != ""){
        //if(selectedStation != ""){
            populateParameterComboBox(selectedStation);
        //}
        if(selectedParam != "Wybierz parametr powietrza" &&
            selectedParam != ""){
            try{
                const string selectedTime = ui->timeSelect->currentText().toStdString();
                displayChart(myFile_->getDataPoints(selectedStation.toStdString(), selectedParam.toStdString(), selectedTime), selectedStation, selectedParam);
            }catch(const exception& e){
                QMessageBox::critical(this, qstr(_("Błąd", lang_)), qstr(e.what()));
            }

        }
    }
}
