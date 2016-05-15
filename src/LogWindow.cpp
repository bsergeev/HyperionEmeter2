#include "LogWindow.h"

#include "HypReader.h"

#include <QtWidgets>

LogWindow::LogWindow(QWidget* parent)
    : QTextEdit(parent)
{
    //setAttribute(Qt::WA_DeleteOnClose);
}

void LogWindow::AddLine(const QString& text)
{
    QString message = text;
    if (text[0] == HypReader::BS) {
        moveCursor(QTextCursor::End,         QTextCursor::MoveAnchor);
        moveCursor(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        moveCursor(QTextCursor::Up,          QTextCursor::KeepAnchor);
        textCursor().removeSelectedText();

        message.remove(0, 1);
    }
    append(message);
}
