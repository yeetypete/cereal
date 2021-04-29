
#include "console.h"

#include <QScrollBar>
#include <QDebug>

Console::Console(QWidget *parent) :
  QPlainTextEdit(parent) {
  document()->setMaximumBlockCount(100);
  QPalette p = palette();
  p.setColor(QPalette::Base, Qt::black);
  p.setColor(QPalette::Text, Qt::white);
  p.setColor(QPalette::Highlight, Qt::white);
  p.setColor(QPalette::HighlightedText, Qt::black);
  setPalette(p);
  insertPlainText("cereal > ");
  min_pos = textCursor().position();

}

void Console::putData(const QByteArray &data) {
  const QTextCursor old_cursor = textCursor();
  const int old_scrollbar_value = verticalScrollBar()->value();
  const bool is_scrolled_down =  old_scrollbar_value == verticalScrollBar()->maximum();
  moveCursor(QTextCursor::End);
  insertPlainText(data);

  if (old_cursor.hasSelection() || !is_scrolled_down) {
    setTextCursor(old_cursor);
    verticalScrollBar()->setValue(old_scrollbar_value);
  } else {
    moveCursor(QTextCursor::End);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
  }
}

void Console::setLocalEchoEnabled(bool set) {
  m_localEchoEnabled = set;
}

void Console::setAutoscrollEnabled(bool set) {
  m_autoscrollEnabled = set;
}

void Console::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Backspace:
    if (textCursor().position() <= min_pos) {
      break;
    } else {
      textCursor().deletePreviousChar();
      usr_input.chop(1);
    }
  case Qt::Key_Left:
  case Qt::Key_Right:
  case Qt::Key_Up:
  case Qt::Key_Down:
    break;
  case Qt::Key_Return: {
    usr_input.remove("\r");
    emit sendCommand(usr_input);
    //emit getData(usr_input.toLocal8Bit());
    usr_input.clear();
    textCursor().insertText("\ncereal > ");
    QTextCursor cursor2 = textCursor();
    cursor2.setPosition(toPlainText().length() - 1, QTextCursor::MoveAnchor);
    setTextCursor(cursor2);
    min_pos = textCursor().position() + 1;
    m_newcommand = true;
  }
  default:
    if (!textCursor().atEnd()) {
      QTextCursor cursor = textCursor();
      cursor.setPosition(toPlainText().length(), QTextCursor::MoveAnchor);
      setTextCursor(cursor);
    }

    QPlainTextEdit::keyPressEvent(e);
    QString key = e->text();
    usr_input.append(key);
    //emit getData(e->text().toLocal8Bit());
  }
  if (m_newcommand) {
    textCursor().deletePreviousChar();
    m_newcommand = false;
  }
}

void Console::mouseMoveEvent(QMouseEvent *e) {
  QPlainTextEdit::mouseMoveEvent(e);
  if (e->buttons() & Qt::LeftButton) {
    QTextCursor cursor = textCursor();
    QTextCursor endCursor = cursorForPosition(e->pos());
    cursor.setPosition(pos, QTextCursor::MoveAnchor);
    cursor.setPosition(endCursor.position(), QTextCursor::KeepAnchor);
    setTextCursor(cursor);
  }
}

void Console::mousePressEvent(QMouseEvent *e) {
  QPlainTextEdit::mousePressEvent(e);
  if (e->buttons() & Qt::LeftButton) {
    QTextCursor cursor = cursorForPosition(e->pos());
    pos = cursor.position();
    cursor.clearSelection();
    setTextCursor(cursor);
  }
  //Q_UNUSED(e)
  //setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *e) {
  Q_UNUSED(e)
}

void Console::contextMenuEvent(QContextMenuEvent *e) {
  Q_UNUSED(e)
}

void Console::commandOutput(QString result) {
  insertPlainText("\n" + result);
}

