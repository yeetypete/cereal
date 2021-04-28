/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
    m_chart->resize(400,70);
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

MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_ui;
}

//! [4]
void MainWindow::openSerialPort()
{
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
void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    //m_console->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    const QByteArray data = m_serial->readAll();
    m_console->putData(data);
    m_chart->parseSerial(data);
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }

}
//! [8]

void MainWindow::initActionsConnections()
{
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

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::showTerminal()
{
    m_ui->dockWidget_2->show();
}

void MainWindow::showPlotter()
{
    m_ui->dockWidget->show();
}

void MainWindow::runCommand(QString command)
{
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
                if (file.open(QIODevice::ReadWrite)){
                    QTextStream stream(&file);
                    stream << m_console->toPlainText();
                    file.flush();
                    file.close();
                } else {
                    response = "cterror: error saving to file";
                    emit commandResponse(response);
                }
            }
        } else {
            response = tr("cterror: invalid command: %1").arg(list1.at(0));
            emit commandResponse(response);
        }
    }
}
