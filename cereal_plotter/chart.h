#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>
#include <QtCore/QTimer>
#include <QVector>
#include <QRegExp>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;

QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

//![1]
class SerialSignal {
 public:
  QPen m_pen;
  QLineSeries *m_series;
};

class Chart: public QChart {
  Q_OBJECT
 public:
  Chart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);

 public slots:
  void generateSignal();
 public:
  void autoScrollX(qreal);
  void autoScaleY(qreal);
  void parseSerial();
  void eraseNotDisplayed();
  void dynamicAxisX(qreal);

 private:
  QVector<SerialSignal> m_SerialSingals;
  QTimer m_timer;
  qreal m_time_x;
  QValueAxis *m_axisX;
  QValueAxis *m_axisY;
  qreal m_maxY;
  qreal m_minY;
  QVector<QPoint> m_points;
  qreal m_t_interval;
};

//![1]

#endif /* CHART_H */
