#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>
#include <QtCore/QTimer>
#include <QVector>
#include <QElapsedTimer>

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
  void autoScaleY(qreal, bool);
  void eraseNotDisplayed();
  void dynamicAxisX(qreal);
  void indicatorAxisX();

 private:
  QVector<SerialSignal> m_SerialSingals;
  QElapsedTimer m_timer_x;
  QValueAxis *m_axisX;
  QValueAxis *m_axisY;
  QVector<QPoint> m_points;
  qreal m_newlines;
  bool m_timeDomain;
  bool m_data_recieved;
  QStringList m_delimiters;
};

//![2]

#endif /* CHART_H */
