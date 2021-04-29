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
  m_timeDomain(true),
  m_autoYScaling(true),
  m_axisYSymmetric(false),
  m_axisYSmooth(false),
  m_axisYSmooth_mult(DEFAULT_SMOOTH),
  m_axisX(new QValueAxis()),
  m_axisY(new QValueAxis()),
  m_newlines(0),
  m_data_recieved(false),
  m_delimiters{"\t", ";", ",", " "} {

  addAxis(m_axisX, Qt::AlignBottom);
  addAxis(m_axisY, Qt::AlignLeft);

  m_axisX->setTickCount(DEFAULT_X_TICKS);
  m_axisY->setTickCount(DEFAULT_Y_TICKS);
  m_axisX->setRange(0, TDOMAIN_RANGE);
  m_axisY->setRange(-DEFAULT_Y_RANGE, DEFAULT_Y_RANGE);

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

void Chart::autoScaleY(qreal offset_pcnt) {
  // find max peaks in series
  if (!m_autoYScaling)
    return;
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

  if (m_axisYSymmetric) {
    view_maxY = qMax(qAbs(view_maxY), qAbs(view_minY));
    view_minY = - view_maxY;
  }

  if (!m_axisYSmooth) {
  if (view_minY < offset_pcnt * m_axisY->min()) {
    m_axisY->setMin(view_minY * (1 / offset_pcnt));
  } else if (view_minY * offset_pcnt > m_axisY->min()) {
      m_axisY->setMin(view_minY * (1/offset_pcnt));
    }

  if (view_maxY > offset_pcnt * m_axisY->max()) {
      m_axisY->setMax(view_maxY * (1/offset_pcnt));
  } else if (view_maxY * offset_pcnt < m_axisY->max()) {
      m_axisY->setMax(view_maxY * (1/offset_pcnt));
    }
  }

  // smooth scaling
  else {
      if (view_minY < offset_pcnt * m_axisY->min()) {
            m_axisY->setMin(view_minY * (1/offset_pcnt));
        }
        else {
            qreal rate = 0;
            if (m_axisY->min() != 0) {
              rate = (m_axisY->min() * offset_pcnt - view_minY) / m_axisY->min();
              rate *= m_axisYSmooth_mult;
            }
            m_axisY->setMin(m_axisY->min() + rate);
        }
        if (view_maxY > offset_pcnt * m_axisY->max()) {
            m_axisY->setMax(view_maxY * (1/offset_pcnt));
        }
        else {
            qreal rate = 0;
            if (m_axisY->max() != 0) {
                    rate = (m_axisY->max() * offset_pcnt - view_maxY) / m_axisY->max();
                    rate *= m_axisYSmooth_mult;
            }
            m_axisY->setMax(m_axisY->max() - rate);
        }
  }
}


void Chart::parseSerial(const QByteArray &byte_data) {
  QString string_data = QString(byte_data.trimmed());
  m_newlines ++;
  plotSerialSignals(string_data);
  autoScrollX(0.95);
  autoScaleY(0.95);
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

  else if (contents.size() <= m_SerialSingals.size()) {
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
  } else {
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

    QString s_name = "signal_";
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
      //qDebug() << "series size: " << SSignal.m_series->pointsVector().size();
    }
  }
}

void Chart::clearChart() {
  m_SerialSingals.clear();
  m_axisX->setRange(0,TDOMAIN_RANGE);
  m_axisY->setRange(-DEFAULT_Y_RANGE, DEFAULT_Y_RANGE);
  removeAllSeries();
  m_data_recieved = false;
  m_timer_x.invalidate();
}

void Chart::setXRange(qreal range) {
  m_axisX->setRange(m_axisX->min(), m_axisX->min() + range);
}

void Chart::setYRange(qreal min, qreal max) {
  m_axisY->setRange(min, max);
}

void Chart::setTimeDomain() {

}

void Chart::setSampleDomain() {

}

bool Chart::rename(QString current_name, QString new_name) {
    int matches_current = 0;
    int match_index = 0;

    // TODO: add support for whitespace in name
    //qDebug() << "rename called";

//    if (current_name.contains(" ")) {
//            if (current_name.at(0) != "\"" && current_name.at(current_name.size() - 1) != "\"") {
//                qDebug() << current_name;
//                return false;
//            }
//            else {
//                current_name.remove(0,1);
//                current_name.remove(current_name.size() - 1, 1);
//            }
//    }


//    if (new_name.contains(" ")) {
//            if (new_name.at(0) != "\"" && new_name.at(new_name.size() - 1) != "\"") {
//                return false;
//                qDebug() << new_name;
//            }
//            else {
//                new_name.remove(0,1);
//                new_name.remove(new_name.size() - 1, 1);
//            }
//    }

    for (int i = 0; i < m_SerialSingals.size(); i++) {
        if (m_SerialSingals.at(i).m_series->name().contains(current_name)) {
            matches_current ++;
            match_index = i;
        }
        if (m_SerialSingals.at(i).m_series->name() == new_name) {
            return false;
        }
    }
    if (matches_current == 1) {
        m_SerialSingals.at(match_index).m_series->setName(new_name);
        legend()->update();
        return true;
    }
    else
        return false;
}

