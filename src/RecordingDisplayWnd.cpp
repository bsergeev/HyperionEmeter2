#include "RecordingDisplayWnd.h"

#include "Recording.h"
#include "RecordingDisplayOptionsDlg.h"
#include "RecordingDataModel.h"
#include "RecordingTableView.h"
#include "RecordingPlotter.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QSplitter>
#include <QVBoxLayout>

#include <cassert>

RecordingDisplayWnd::RecordingDisplayWnd(std::shared_ptr<RecordingDataModel> dataModel,
                                         QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_dataModel(dataModel)
    , m_Plotter(std::make_unique<RecordingPlotter>  (dataModel, this))
    , m_Table  (std::make_unique<RecordingTableView>(dataModel, this))
{
    if (RecordingDataModel* const model = m_dataModel.get()) {
        const Recording& recording = model->GetRecording();
        setWindowTitle(QString::fromStdString(recording.GetTitle()));
    }

    setBackgroundRole(QPalette::Light);
    setAttribute(Qt::WA_DeleteOnClose);

    QItemSelectionModel* selectionModel = new QItemSelectionModel(m_dataModel.get(), this);
    m_Plotter->setSelectionModel(selectionModel);
    m_Table->setSelectionModel(selectionModel);
    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            m_Plotter->viewport(), SLOT(update()));

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

    m_graphOptions_Actn = new QAction(tr("Display options"), this);
    connect(m_graphOptions_Actn, &QAction::triggered, [this]{ SetGraphOptions(); });
}

void RecordingDisplayWnd::ResizeTable()
{
    if (m_Table != nullptr) {
        m_Table->resizeColumnsToContents();
        int w = m_Table->GetTotalWidth();
        QList<int> widths;   widths << w << m_Splitter->width() - w;
        m_Splitter->setSizes(widths);
    }
}

void RecordingDisplayWnd::SetGraphOptions()
{
    RecordingDisplayOptionsDlg dlg(true, true, m_dataModel.get(), m_Plotter.get(), m_Table.get(), 0,
                                   false, RecordingPlotter::sShowTitle, RecordingPlotter::sShowSubTitle);
    if (dlg.exec() == QDialog::Accepted)
    {
        // kShowLegend = dlg.GetShowLegend();
        RecordingPlotter::sShowTooltip = dlg.GetShowTooltip();

        //psMgr.ChangeXAxisType(MeasurementPlotSettingsMgr::kXAxisType = (XAxisType)dlg.GetXAxisType());
        RecordingTableView::sColorColumns = dlg.GetClrColumns();

        const Recording& recording = m_dataModel->GetRecording();
        const size_t N_curves = recording.numColums();

        for (size_t crvIdx = 1; crvIdx < N_curves; ++crvIdx)
        {
            m_dataModel->SetColumnColor(crvIdx, dlg.GetCurveColor(crvIdx));
            m_Plotter->SetCurveVisible (crvIdx, dlg.GetCurveVisible(crvIdx));
            m_Table->SetColumnVisible  (crvIdx, dlg.GetColumnVisible(crvIdx));
        }
        RecordingPlotter::kGraphBkgrColor  = dlg.GetBackgroundColor();
        RecordingPlotter::kGraphFrameColor = dlg.GetFrameColor();
        RecordingPlotter::kGraphGridColor  = dlg.GetGridlineColor();


        m_Plotter->SetShowTitle   (RecordingPlotter::sShowTitle = dlg.GetShowTitle());
        m_Plotter->SetShowSubTitle(RecordingPlotter::sShowSubTitle = dlg.GetShowSubTitle());

        //kWindowStyle = (!dlg.IsTableEnabled()) ? kGraphOnly
        //	: (!dlg.IsPlotEnabled()) ? kTableOnly
        //	: kTableAndGraph;
        //if (kWindowStyle != thisWndStyle)
        //	ChangeWindowStyle(kWindowStyle);

        //m_Table->UpdateColumnVisibility();
        ResizeTable();

        if (m_Plotter.get()) {
            m_Plotter->AdjustScrMargins();
            m_Plotter->ComputeTicks();
        }
    }
}

void RecordingDisplayWnd::resizeEvent(QResizeEvent* evt)
{
    QWidget::resizeEvent(evt);
    ResizeTable();
}

void RecordingDisplayWnd::contextMenuEvent(QContextMenuEvent* evt)
{
    QMenu menu(this);

    menu.addAction(m_graphOptions_Actn);
    //menu.addAction(m_EditTitles_Act);

    menu.exec(evt->globalPos());
}
