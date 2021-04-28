#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>
#include <QtCore/QTimer>
#include <QVector>
#include <QElapsedTimer>

#define TDOMAIN_RANGE 5000
#define SDOMAIN_RANGE 100
#define DEFAULT_Y_RANGE 10

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;

QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

//![1]
class SerialSignal {
 public:
  SerialSignal();
  QPen m_pen;
  QLineSeries *m_series;
  qreal m_num_points;
};
//![1]

//![2]
class Chart: public QChart {
  Q_OBJECT
 public:
  Chart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);

 public slots:
  void parseSerial(const QByteArray &data);

 public:
  void plotSerialSignals(QString);
  void dynamicAddSeries(int);
  void autoScrollX(qreal);
  void autoScaleY(qreal);
  void eraseNotDisplayed();
  void dynamicAxisX(qreal);
  void clearChart();
  void setTimeDomain();
  void setSampleDomain();
  void setXRange(qreal);
  void setYRange(qreal, qreal);

 public:
  bool m_timeDomain;
  bool m_autoYScaling;
  bool m_axisYSymmetric;
  bool m_axisYSmooth;

 private:
  QVector<SerialSignal> m_SerialSingals;
  QElapsedTimer m_timer_x;
  QValueAxis *m_axisX;
  QValueAxis *m_axisY;
  QVector<QPoint> m_points;
  qreal m_newlines;
  bool m_data_recieved;
  QStringList m_delimiters;
};

//![2]

#endif /* CHART_H */
