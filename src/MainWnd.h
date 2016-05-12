#pragma once

#include "HypReader.h"

#include <QtWidgets/QMainWindow>
#include <memory>

class QWidget;
class QMdiArea;

//------------------------------------------------------------------------------

class MainWnd : public QMainWindow
{
    Q_OBJECT

public:
    MainWnd(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~MainWnd() {}

private:
    void updateMenus();

//data:
    std::unique_ptr<HypReader> m_reader;

    QMdiArea* m_mdiArea;

};