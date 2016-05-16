#pragma once

#include <QTextEdit>

class LogWindow: public QTextEdit
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget* parent = 0);

    void AddLine(const QString& text);

//protected:
//    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
};
