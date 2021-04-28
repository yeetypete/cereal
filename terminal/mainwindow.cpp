#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "chart.h"

#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QTextStream>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  m_ui(new Ui::MainWindow),
  m_status(new QLabel),
  m_console(new Console),
  m_terminal(new Console),
  m_settings(new SettingsDialog),
//! [1]
  m_serial(new QSerialPort(this)),
  m_chart(new Chart)

//! [1]
{
//! [0]
  m_ui->setupUi(this);
  m_console->setEnabled(false);
  m_console->setReadOnly(true);
  m_console->clear();
  m_terminal->setEnabled(true);
  m_terminal->setLocalEchoEnabled(true);
  m_ui->dockWidget_2->setAllowedAreas(Qt::BottomDockWidgetArea);
  m_ui->dockWidget_2->setWidget(m_terminal);


  setCentralWidget(m_console);

  m_ui->actionConnect->setEnabled(true);
  m_ui->actionDisconnect->setEnabled(false);
  m_ui->actionQuit->setEnabled(true);
  m_ui->actionConfigure->setEnabled(true);

  m_ui->statusBar->addWidget(m_status);

  m_chart->legend()->show();
  m_chart->setAnimationOptions(QChart::NoAnimation);
  m_chart->resize(400, 70);
  QChartView *chartView = new QChartView(m_chart);
  chartView->setRenderHint(QPainter::Antialiasing);

  m_ui->dockWidget->setWidget(chartView);
  m_ui->dockWidget->show();

  initActionsConnections();

  connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

//! [2]
  connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
//! [2]
  connect(m_console, &Console::getData, this, &MainWindow::writeData);
  connect(m_terminal, &Console::getData, this, &MainWindow::writeData);
  connect(m_terminal, &Console::sendCommand, this, &MainWindow::runCommand);
  connect(this, &MainWindow::commandResponse, m_terminal, &Console::commandOutput);


//! [3]
}
//! [3]

MainWindow::~MainWindow() {
  delete m_settings;
  delete m_ui;
}

//! [4]
void MainWindow::openSerialPort() {
  const SettingsDialog::Settings p = m_settings->settings();
  m_serial->setPortName(p.name);
  m_serial->setBaudRate(p.baudRate);
  m_serial->setDataBits(p.dataBits);
  m_serial->setParity(p.parity);
  m_serial->setStopBits(p.stopBits);
  m_serial->setFlowControl(p.flowControl);
  if (m_serial->open(QIODevice::ReadWrite)) {
    m_console->setEnabled(true);
    m_console->setLocalEchoEnabled(p.localEchoEnabled);
    m_ui->actionConnect->setEnabled(false);
    m_ui->actionDisconnect->setEnabled(true);
    m_ui->actionConfigure->setEnabled(false);
    showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                      .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                      .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
  } else {
    QMessageBox::critical(this, tr("Error"), m_serial->errorString());

    showStatusMessage(tr("Open error"));
  }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort() {
  if (m_serial->isOpen())
    m_serial->close();
  //m_console->setEnabled(false);
  m_ui->actionConnect->setEnabled(true);
  m_ui->actionDisconnect->setEnabled(false);
  m_ui->actionConfigure->setEnabled(true);
  showStatusMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about() {
  QMessageBox::about(this, tr("About Simple Terminal"),
                     tr("<b>Cereal</b> Version 1.0 © 2021 Dylan Derose, Peter Siegel"));
}

//! [6]
void MainWindow::writeData(const QByteArray &data) {
  m_serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData() {
  const QByteArray data = m_serial->readLine();
  m_console->putData(data);
  m_chart->parseSerial(data);
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error) {
  if (error == QSerialPort::ResourceError) {
    QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
    closeSerialPort();
  }

}
//! [8]

void MainWindow::initActionsConnections() {
  connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
  connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
  connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
  connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
  connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
  connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
  connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
  connect(m_ui->actionTerminal, &QAction::triggered, this, &MainWindow::showTerminal);
  connect(m_ui->actionPlotter, &QAction::triggered, this, &MainWindow::showPlotter);

}

void MainWindow::showStatusMessage(const QString &message) {
  m_status->setText(message);
}

void MainWindow::showTerminal() {
  m_ui->dockWidget_2->show();
}

void MainWindow::showPlotter() {
  m_ui->dockWidget->show();
}

void MainWindow::runCommand(QString command) {
  qDebug() << "recieved command: " << command << "\n";
  QString recievedCommand = command;
  QString response;
  QStringList list1 = recievedCommand.split(" ");
  if (list1.length() > 0) {
    if (list1.at(0) == "connect" || list1.at(0) == "c") {
      if (!m_serial->isOpen()) {
        openSerialPort();
      } else {
        response = "cterror: serial already connected";
        emit commandResponse(response);
      }
    } else if (list1.at(0) == "disconnect" || list1.at(0) == "dc") {
      if (m_serial->isOpen()) {
        closeSerialPort();
        m_chart->clearChart();
      } else {
        response = "cterror: serial not connected";
        emit commandResponse(response);
      }
    } else if (list1.at(0) == "baud") {
      m_serial->setBaudRate(list1.at(1).toInt());
      m_settings->setBaud(list1.at(1).toInt());
      if (m_serial->isOpen()) {
        const SettingsDialog::Settings p = m_settings->settings();
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(list1.at(1)).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
      }
    } else if (list1.at(0) == "clear") {
      m_terminal->clear();
    } else if (list1.at(0) == "port") {
      m_serial->setPortName(list1.at(1));
    } else if (list1.at(0) == "send") {
      writeData(command.midRef(list1.at(0).length()).toLocal8Bit());
    } else if (list1.at(0) == "save") {
      if (list1.at(1) != "") {
        QFile file(list1.at(1));
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
          QTextStream stream(&file);
          stream << m_console->toPlainText();
          //file.flush();
          file.close();
          emit commandResponse(tr("ct: successfully wrote to file: %1").arg(list1.at(1)));
        } else {
          response = "cterror: error saving to file";
          emit commandResponse(response);
        }
      }
    } else if (list1.at(0) == "tdomain") {
      m_chart->setTimeDomain();
    } else if (list1.at(0) == "sdomain") {
      m_chart->setSampleDomain();
    } else if (list1.at(0) == "xrange") {
      if (list1.size() > 2) {
        emit commandResponse(tr("cterror: xrange takes 1 argument, %1 given").arg(list1.size() - 1));
      }
      if (list1.size() == 1) {
        if (m_chart->m_timeDomain)
          m_chart->setXRange(TDOMAIN_RANGE);
        else
          m_chart->setXRange(SDOMAIN_RANGE);
      } else {
        qreal range = list1.at(1).toDouble();
        m_chart->setXRange(range);
      }
    } else if (list1.at(0) == "yrange") {
      if (list1.size() == 1) {
          response = tr("cterror: invalid command: %1").arg(list1.at(0));
          emit commandResponse(response);
          return;
      }
      if (list1.size() == 2) {
        if (list1.at(1) == "auto") {
          m_chart->m_autoYScaling = true;
          return;
        } else if (list1.at(1) == "manual") {
          m_chart->m_autoYScaling = false;
          return;
        } else if (list1.at(1) == "mirror") {
          m_chart->m_axisYSymmetric = true;
          return;
        } else if (list1.at(1) == "nmirror") {
            m_chart->m_axisYSymmetric = false;
            return;
        } else if (list1.at(1) == "smooth") {
          m_chart->m_axisYSmooth = true;
          m_chart->m_autoYScaling = true;
          return;
        } else if (list1.at(1) == "nsmooth") {
            m_chart->m_axisYSmooth = false;
            return;
        } else {
            emit commandResponse(tr("cterror: yrange does not take this argument."));
        }
      }
      if (list1.size() == 3) {
        m_chart->m_autoYScaling = false;
        qreal min = list1.at(1).toDouble();
        qreal max = list1.at(2).toDouble();
        m_chart->setYRange(min, max);
      }
      else {
          response = tr("cterror: invalid command: %1").arg(list1.at(0));
          emit commandResponse(response);
          return;
      }
    } else {
      response = tr("cterror: invalid command: %1").arg(list1.at(0));
      emit commandResponse(response);
    }
  }
}
