#include "RecordingDisplayWnd.h"

#include "RecordingTableView.h"
#include "RecordingPlotter.h"

#include <QSplitter>
#include <QVBoxLayout>

RecordingDisplayWnd::RecordingDisplayWnd(std::unique_ptr<RecordingTableView> table, 
                                         std::unique_ptr<RecordingPlotter>   plotter,
                                         QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_Plotter(std::move(plotter))
    , m_Table  (std::move(table))
{
    setBackgroundRole(QPalette::Light);
    setAttribute(Qt::WA_DeleteOnClose);

    if (QVBoxLayout* layout = new (std::nothrow) QVBoxLayout(this))
    {
        m_Splitter = new QSplitter(this);
        if (m_Table) {
            m_Table->resizeColumnsToContents();
            m_Splitter->addWidget(m_Table.get());
        }
        if (m_Plotter) {
            m_Splitter->addWidget(m_Plotter.get());
        }
        layout->addWidget(m_Splitter);
    }
}


