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

#include "console.h"

#include <QScrollBar>
#include <QDebug>

Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
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

void Console::putData(const QByteArray &data)
{
    const QTextCursor old_cursor = textCursor();
    const int old_scrollbar_value = verticalScrollBar()->value();
    const bool is_scrolled_down =  old_scrollbar_value == verticalScrollBar()->maximum();
    moveCursor(QTextCursor::End);
    insertPlainText(data);

    if (old_cursor.hasSelection() || !is_scrolled_down)
    {
        setTextCursor(old_cursor);
        verticalScrollBar()->setValue(old_scrollbar_value);
    }
    else
    {
        moveCursor(QTextCursor::End);
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    }
}

void Console::setLocalEchoEnabled(bool set)
{
    m_localEchoEnabled = set;
}

void Console::setAutoscrollEnabled(bool set)
{
    m_autoscrollEnabled = set;
}

void Console::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Backspace:
        if (textCursor().position() <= min_pos)
        {
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
    case Qt::Key_Return:
    {
        usr_input.remove("\r");
        emit sendCommand(usr_input);
        //emit getData(usr_input.toLocal8Bit());
        qDebug() << "command entered: " << usr_input << "\n";
        usr_input.clear();
        textCursor().insertText("\ncereal > ");
        //appendPlainText("cereal > ");
        QTextCursor cursor2 = textCursor();
        cursor2.setPosition(toPlainText().length()-1, QTextCursor::MoveAnchor);
        setTextCursor(cursor2);
        min_pos = textCursor().position() + 1;
        m_newcommand = true;
    }
    default:
        if (!textCursor().atEnd())
        {
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

void Console::mouseMoveEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseMoveEvent(e);
    if (e->buttons() & Qt::LeftButton)
    {
        QTextCursor cursor = textCursor();
        QTextCursor endCursor = cursorForPosition(e->pos());
        cursor.setPosition(pos, QTextCursor::MoveAnchor);
        cursor.setPosition(endCursor.position(), QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
}

void Console::mousePressEvent(QMouseEvent *e)
{
    QPlainTextEdit::mousePressEvent(e);
    if (e->buttons() & Qt::LeftButton)
    {
        QTextCursor cursor = cursorForPosition(e->pos());
        pos = cursor.position();
        cursor.clearSelection();
        setTextCursor(cursor);
    }
    //Q_UNUSED(e)
    //setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}

void Console::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e)
}

void Console::commandOutput(QString result)
{
    qDebug() << "Recieved response: " << result << "\n";
    insertPlainText("\n" + result);
}

