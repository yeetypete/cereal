
#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>

class Console : public QPlainTextEdit {
  Q_OBJECT

 signals:
  void getData(const QByteArray &data);
  void sendCommand(QString command);

 public:
  explicit Console(QWidget *parent = nullptr);

  void putData(const QByteArray &data);
  void setLocalEchoEnabled(bool set);
  void setAutoscrollEnabled(bool set);
 public slots:
  void commandOutput(QString result);

 protected:
  void keyPressEvent(QKeyEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;
  void mouseDoubleClickEvent(QMouseEvent *e) override;
  void contextMenuEvent(QContextMenuEvent *e) override;

 private:
  bool m_localEchoEnabled = false;
  bool m_autoscrollEnabled = true;
  bool m_newcommand = false;
  QString usr_input;
  int min_pos;
  int pos;
  int sliderpos;
};

#endif // CONSOLE_H
