#include "chart.h"
#include <QtCharts/QChartView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>

QT_CHARTS_USE_NAMESPACE

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  QMainWindow window;
  Chart *chart = new Chart;
  //chart->setTitle("Cereal (SPECIFY PORT & INFO HERE)");
  chart->legend()->show();
  chart->setAnimationOptions(QChart::NoAnimation);
  QChartView chartView(chart);
  chartView.setRenderHint(QPainter::Antialiasing);
  window.setCentralWidget(&chartView);
  window.resize(800, 600);
  window.show();
  return a.exec();
}
