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

#define _(phrase, lang) string(Translate((phrase), (lang)))
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

void mainwindow::displayChart(const QVector<QPointF> &points, const QString &title) {
    if (points.isEmpty()) {
        QMessageBox::critical(this, qstr(_("Błąd", lang_)), qstr(_("Brak pomiarów dla podanych parametrów", lang_)));
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
    axisX->setTitleText(qstr(_("Czas", lang_)));

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText(qstr(_("Wartość", lang_)));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chartView->setChart(chart);
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
                QString title;
                if(selectedStation == "Średnia wszystkich stacji"){
                    title = qstr(_("Uśrednione pomiary ", lang_)) + qstr(_(selectedParam.toStdString(), lang_)) + qstr(_(" w mieście ", lang_)) + myFile_->getCity();
                }else{
                    title = qstr(_(selectedParam.toStdString(), lang_)) + qstr(_(" w okolicy ", lang_)) + selectedStation;
                }
                displayChart(myFile_->getDataPoints(selectedStation.toStdString(), selectedParam.toStdString()), title);
            }catch(const exception& e){
                QMessageBox::critical(this, qstr(_("Błąd", lang_)), qstr(e.what()));
            }

        }
    }
}
