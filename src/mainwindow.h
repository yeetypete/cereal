
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class Chart;
class SettingsDialog;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 signals:
  void commandResponse(QString output);

 private slots:
  void openSerialPort();
  void closeSerialPort();
  void about();
  void writeData(const QByteArray &data);
  void readData();
  void showTerminal();
  void showPlotter();
  void runCommand(QString command);

  void handleError(QSerialPort::SerialPortError error);

 private:
  void initActionsConnections();

 private:
  void showStatusMessage(const QString &message);

  Ui::MainWindow *m_ui = nullptr;
  QLabel *m_status = nullptr;
  Console *m_console = nullptr;
  Console *m_terminal = nullptr;
  SettingsDialog *m_settings = nullptr;
  QSerialPort *m_serial = nullptr;
  Chart *m_chart = nullptr;
};

#endif // MAINWINDOW_H
