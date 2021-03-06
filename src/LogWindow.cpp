#include "LogWindow.h"

#include "HypReader.h"

//#pragma warning(disable:4127)
//#include <QtWidgets>
//#pragma warning(default:4127)
#include <QIcon>
#include <QString>
#include <QTextCursor>

LogWindow::LogWindow(QWidget* parent)
    : QTextEdit(parent)
{
    //setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Log Window"));
    setWindowIcon(QIcon(":/images/face-smile.png"));
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
