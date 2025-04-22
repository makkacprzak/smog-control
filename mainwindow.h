#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "jfile.h"
#include <QtCharts/QChartView>

QT_BEGIN_NAMESPACE
namespace Ui {
class mainwindow;
}
QT_END_NAMESPACE

class mainwindow : public QMainWindow
{
    Q_OBJECT
public:
    QMap<QString, int> stationMap;
    mainwindow(QWidget *parent = nullptr);
    ~mainwindow();
    std::string getTextBox();
    void populateStationComboBox();
    void populateParameterComboBox(const QString& station);
    void displayChart(const QVector<QPointF> &points, const QString &title);
    void checkAndDrawChart();
public slots:
    void on_searchBtn_clicked();

private:
    Ui::mainwindow *ui;
    Jfile* myFile_ = nullptr;
    QChartView* chartView;
};


#endif // MAINWINDOW_H
