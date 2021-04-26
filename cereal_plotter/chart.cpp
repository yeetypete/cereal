#include "chart.h"
#include <QtCharts/QAbstractAxis>

#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>

#include <QtCharts/QValueAxis>
#include <QtCore/QRandomGenerator>
#include <QtCore/QDebug>
#include <QEasingCurve>
#include <limits>

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
  QChart(QChart::ChartTypeCartesian, parent, wFlags),
  m_time_x(0),
  m_axisX(new QValueAxis()),
  m_axisY(new QValueAxis()),
  m_maxY(0),
  m_minY(0){
  QObject::connect(&m_timer, &QTimer::timeout, this, &Chart::generateSignal);
  m_timer.setInterval(10);

  addAxis(m_axisX, Qt::AlignBottom);
  addAxis(m_axisY, Qt::AlignLeft);

  m_axisX->setTickCount(10);
  m_axisX->setRange(0, 100);
  m_axisY->setRange(-10, 10);

  //Initialize Serial Signals
  for (int i = 0; i < 2; i++) {
    SerialSignal *SSignal = new SerialSignal;
    SSignal->m_series = new QLineSeries;

    //Set pen
//    SSignal->m_pen.setColor(Qt::red);
//    SSignal->m_pen.setWidth(3);
//    SSignal->m_series->setPen(SSignal->m_pen);

    //Set name for legend
    QString s_name = "Signal ";
    s_name += QString::number(i);
    SSignal->m_series->setName(s_name);

    //Attach axes
    addSeries(SSignal->m_series);
    SSignal->m_series->attachAxis(m_axisX);
    SSignal->m_series->attachAxis(m_axisY);
//    SSignal->m_series->setUseOpenGL(true);

    m_SerialSingals.push_back(*SSignal);
  }

  m_timer.start();
  m_t_interval = m_timer.interval();
  legend()->setVisible(true);
  legend()->setAlignment(Qt::AlignTop);
  legend()->attachToChart();
  setAnimationDuration(m_t_interval);
}

void Chart::generateSignal() {
  m_time_x += m_timer.interval();
  for (auto & SSignal : m_SerialSingals) {
    qreal noise = QRandomGenerator::global()->bounded(10) - 5;
    qreal rand_y = noise;
//    if (m_time_x / 100 - int(m_time_x /100) < 1) {
//        rand_y += 100;
//    }
    SSignal.m_series->append(m_time_x, rand_y);
  }
  dynamicAxisX(100); // number of samples to display in time-series
  autoScrollX(0.95); // percent of chart
  eraseNotDisplayed();
  //autoScaleY(0.9);
}

void Chart::autoScrollX(qreal offset_pcnt) {
  // convert between pixel and axis units
  qreal axis_x_width_dec = m_axisX->max() - m_axisX->min();
  qreal axis_x_width_pixels = plotArea().width();

  qreal delta_x = axis_x_width_pixels / axis_x_width_dec * m_t_interval;
  qreal offset_x = axis_x_width_dec * offset_pcnt + m_axisX->min();
  bool past_offset = false;

  for (auto & SSignal : m_SerialSingals) {
    QPointF last = SSignal.m_series->pointsVector().constLast();
    if (offset_x - last.x() <= 0)
      past_offset = true;
  }

  if (past_offset) {
    scroll(delta_x, 0);
    //qDebug() << "delta_x:" << delta_x;
  }
}

void Chart::dynamicAxisX(qreal num_points) {
  if (m_t_interval != 0) {
    m_axisX->setRange(m_axisX->min(),
                      m_axisX->min() + num_points * m_t_interval);
  }
}

void Chart::autoScaleY(qreal offset_pcnt) {
  // find max peaks in series
  for (auto & SSignal : m_SerialSingals) {
    qreal min_y = std::numeric_limits<qreal>::max();
    qreal max_y = std::numeric_limits<qreal>::min();
    for (auto & p : SSignal.m_series->pointsVector()) {
      min_y = qMin(min_y, p.y());
      max_y = qMax(max_y, p.y());
    }
    if (min_y < m_maxY)
        m_maxY = min_y;
    if (max_y > m_minY)
        m_minY = max_y;
  }
  QEasingCurve easingMaxY(QEasingCurve::Linear);
  QEasingCurve easingMinY(QEasingCurve::Linear);
  if (m_maxY > offset_pcnt * m_axisY->max()) {
      qreal axisYMax = easingMaxY.valueForProgress(m_maxY);
      m_axisY->setMax(axisYMax);
  }
  if (m_minY < offset_pcnt * m_axisY->min()) {
      qreal axisYMin = easingMinY.valueForProgress(m_minY);
      m_axisY->setMin(axisYMin);
  }

  qDebug() << "max_y: " << m_maxY;
  qDebug() << "min_y: " << m_minY;
}

void Chart::parseSerial() {

}

void Chart::eraseNotDisplayed() {
  for (auto & SSignal : m_SerialSingals) {
    if (SSignal.m_series->pointsVector().at(0).x() < m_axisX->min()) {
      SSignal.m_series->remove(0);
      //qDebug() << "series size: " << SSignal.m_series->pointsVector().size();
    }
  }
}


