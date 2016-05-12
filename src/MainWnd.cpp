#include "MainWnd.h"

#include <QtWidgets>

MainWnd::MainWnd(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_mdiArea(new QMdiArea)
{
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy  (Qt::ScrollBarAsNeeded);
    setCentralWidget(m_mdiArea);
    connect(m_mdiArea, &QMdiArea::subWindowActivated,
        this, &MainWnd::updateMenus);

    updateMenus();

}

void MainWnd::updateMenus()
{
}