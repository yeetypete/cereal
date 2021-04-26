#include "chart.h"
#include <QtCharts/QAbstractAxis>

#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>

#include <QtCharts/QValueAxis>
//#include <QtCore/QRandomGenerator>
#include <QtCore/QDebug>
#include <QEasingCurve>
#include <limits>
#include <QtMath>

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
  QChart(QChart::ChartTypeCartesian, parent, wFlags),
  m_time_x(0),
  m_axisX(new QValueAxis()),
  m_axisY(new QValueAxis()),
  m_maxY(0),
  m_minY(0){
  QObject::connect(&m_timer, &QTimer::timeout, this, &Chart::generateSignal);
  m_timer.setInterval(50);

  addAxis(m_axisX, Qt::AlignBottom);
  addAxis(m_axisY, Qt::AlignLeft);

  m_axisX->setTickCount(10);
  m_axisY->setTickCount(5);
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
    SSignal->m_series->setUseOpenGL(true);

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
    //qreal noise = QRandomGenerator::global()->bounded(10) - 5;
    qreal rand_y = 0; //noise;
    if (fmod(m_time_x, 500) < 1) {
        rand_y = 100 * sin(m_time_x/5000);
    }
    SSignal.m_series->append(m_time_x, rand_y);
  }
  eraseNotDisplayed();
  dynamicAxisX(50); // number of samples to display in time-series
  autoScrollX(0.95); // percent of chart
  autoScaleY(0.95, false);
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

void Chart::autoScaleY(qreal offset_pcnt, bool symmetric) {
  // find max peaks in series
    qreal view_minY = 0;
    qreal view_maxY = 0;
  for (auto & SSignal : m_SerialSingals) {
    qreal min_y = std::numeric_limits<qreal>::max();
    qreal max_y = std::numeric_limits<qreal>::min();
    for (auto & p : SSignal.m_series->pointsVector()) {
      min_y = qMin(min_y, p.y());
      max_y = qMax(max_y, p.y());
      //qDebug() << "local mins/max: " << min_y << max_y;
    }
    if (min_y < view_minY)
        view_minY = min_y;
    if (max_y > view_maxY)
        view_maxY = max_y;
  }

//  QEasingCurve easingMaxY(QEasingCurve::InExpo);
//  QEasingCurve easingMinY(QEasingCurve::InExpo);
  if (symmetric) {
     view_maxY = qMax(qAbs(view_maxY), qAbs(view_minY));
     view_minY = - view_maxY;
  }
  if (view_minY < offset_pcnt * m_axisY->min()) {
      m_axisY->setMin(view_minY * (1/offset_pcnt));
  }
  else {
      qreal rate = 0;
      if (m_axisY->min() != 0)
        rate = (m_axisY->min() * offset_pcnt - view_minY) / m_axisY->min();
      m_axisY->setMin(m_axisY->min() + rate);
  }
  if (view_maxY > offset_pcnt * m_axisY->max()) {
      m_axisY->setMax(view_maxY * (1/offset_pcnt));
  }
  else {
      qreal rate = 0;
      if (m_axisY->max() != 0)
              rate = (m_axisY->max() * offset_pcnt - view_maxY) / m_axisY->max();
      m_axisY->setMax(m_axisY->max() - rate);
  }
   //axisY->applyNiceNumbers();
  //m_axisY->setTickCount(5);
  //qDebug() << "max_y: " << view_maxY << m_axisY->max();
  //qDebug() << "min_y: " << view_minY << m_axisY->min();
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

void Chart::indicatorAxisX() {

}


