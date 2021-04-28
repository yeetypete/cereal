#include "chart.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCore/QDebug>
#include <QEasingCurve>
#include <limits>
#include <QtMath>
#include <string>

SerialSignal::SerialSignal() {
    m_num_points = 0;
}

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
  QChart(QChart::ChartTypeCartesian, parent, wFlags),
  m_axisX(new QValueAxis()),
  m_axisY(new QValueAxis()),
  m_newlines(0),
  m_data_recieved(false),
  m_delimiters{"\t", ";", ",", " "}{

  addAxis(m_axisX, Qt::AlignBottom);
  addAxis(m_axisY, Qt::AlignLeft);

  m_axisX->setTickCount(10);
  m_axisY->setTickCount(5);
  m_axisX->setRange(0, 5000);
  m_axisY->setRange(-10, 10);

  legend()->setVisible(true);
  legend()->setAlignment(Qt::AlignTop);
  legend()->attachToChart();
}

void Chart::autoScrollX(qreal offset_pcnt) {
  // convert between pixel and axis units
  qreal axis_x_width_dec = m_axisX->max() - m_axisX->min();
  qreal axis_x_width_pixels = plotArea().width();

  qreal offset_x = axis_x_width_dec * offset_pcnt + m_axisX->min();
  qreal max_x = 0;
  for (auto & SSignal : m_SerialSingals) {
      if (SSignal.m_series->pointsVector().isEmpty())
          return;
      if (SSignal.m_series->pointsVector().back().x() > max_x) {
          max_x = SSignal.m_series->pointsVector().back().x();
      }
  }
  qreal delta_x = axis_x_width_pixels / axis_x_width_dec * (max_x - offset_x);
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
      //qDebug() << local mins/max: " << min_y << max_y;
    }
    if (min_y < view_minY)
        view_minY = min_y;
    if (max_y > view_maxY)
        view_maxY = max_y;
  }

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
}


void Chart::parseSerial(const QByteArray &byte_data) {
    QString string_data = QString(byte_data.trimmed());
    m_newlines ++;
    plotSerialSignals(string_data);
    autoScrollX(0.95);
    autoScaleY(0.95, false);
    eraseNotDisplayed();
}

void Chart::plotSerialSignals(QString data_string) {
//    qDebug() << data_string;
    if (!m_timeDomain) {
        m_axisX->setRange(0, 100);
    }

    QStringList contents;
    bool has_delims = false;
    for (auto & delim : m_delimiters) {
        if (data_string.contains(delim)) {
            contents = data_string.split(delim);
            has_delims = true;
        }
    }

    if (!has_delims) {
        contents.push_back(data_string);
    }

    if (contents.isEmpty()) {
        return;
    }

    for (int i = 0; i < contents.size(); i++) {
        if (contents.at(i).isEmpty())
            contents.removeAt(i);
    }

//    qDebug() << contents;

    if (!m_data_recieved) {
        m_data_recieved = true;
        m_timer_x.start();

        dynamicAddSeries(contents.size());

        for (int i = 0; i < contents.size(); i++) {
            qreal sig = contents.at(i).toDouble();
            if (m_timeDomain)
                m_SerialSingals.at(i).m_series->append(m_timer_x.elapsed(), sig);
            else {
                //m_SerialSingals[i].m_num_points ++;
                //m_SerialSingals.at(i).m_series->append(m_SerialSingals.at(i).m_num_points, sig);
                m_SerialSingals.at(i).m_series->append(m_newlines, sig);
            }
        }
    }

    else if (contents.size() <= m_SerialSingals.size()){
        for (int i = 0; i < contents.size(); i++) {
            qreal sig = contents.at(i).toDouble();
            if (m_timeDomain)
                m_SerialSingals.at(i).m_series->append(m_timer_x.elapsed(), sig);
            else {
//                m_SerialSingals[i].m_num_points ++;
//                m_SerialSingals.at(i).m_series->append(m_SerialSingals.at(i).m_num_points, sig);
                  m_SerialSingals.at(i).m_series->append(m_newlines, sig);
            }
        }
    }
    else {
        dynamicAddSeries(contents.size() - m_SerialSingals.size());
        for (int i = 0; i < contents.size(); i++)   {
            qreal sig = contents.at(i).toDouble();
            if (m_timeDomain)
                m_SerialSingals.at(i).m_series->append(m_timer_x.elapsed(), sig);
            else {
//                m_SerialSingals[i].m_num_points ++;
//                m_SerialSingals.at(i).m_series->append(m_SerialSingals.at(i).m_num_points, sig);
                  m_SerialSingals.at(i).m_series->append(m_newlines, sig);
            }
        }
    }
}

void Chart::dynamicAddSeries(int numSeriesToAdd) {
    int numExisting = m_SerialSingals.size();
    for (int i = 0; i < numSeriesToAdd; i++) {
        SerialSignal *SSignal = new SerialSignal;
        SSignal->m_series = new QLineSeries;

        QString s_name = "Signal ";
        s_name += QString::number(i + numExisting);
        SSignal->m_series->setName(s_name);
        addSeries(SSignal->m_series);
        SSignal->m_series->attachAxis(m_axisX);
        SSignal->m_series->attachAxis(m_axisY);
        SSignal->m_series->setUseOpenGL(true);
        m_SerialSingals.push_back(*SSignal);
    }
}

void Chart::eraseNotDisplayed() {
  for (auto & SSignal : m_SerialSingals) {
      if (SSignal.m_series->pointsVector().isEmpty())
          return;
    if (SSignal.m_series->pointsVector().at(0).x() < m_axisX->min()) {
      SSignal.m_series->remove(0);
//      qDebug() << "series size: " << SSignal.m_series->pointsVector().size();
    }
  }
}

void Chart::indicatorAxisX() {

}


