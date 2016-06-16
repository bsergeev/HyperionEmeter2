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
                                         std::unique_ptr<RecordingTableView> table, 
                                         std::unique_ptr<RecordingPlotter>   plotter,
                                         QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_dataModel(dataModel)
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
    RecordingDisplayOptionsDlg dlg(true, true, m_dataModel.get(), m_Plotter.get(), m_Table.get(), 0);
    if (dlg.exec() == QDialog::Accepted)
    {
		// kShowLegend = dlg.GetShowLegend();
		//MeasurementRecordingPlotter::kShowTooltip = dlg.GetShowTooltip();

		//psMgr.ChangeXAxisType(MeasurementPlotSettingsMgr::kXAxisType = (XAxisType)dlg.GetXAxisType());
		//psMgr.ChangeColorColumns(MeasurementPlotSettingsMgr::kColorColumns = dlg.GetClrColumns());
		const Recording& recording = m_dataModel->GetRecording();
		const size_t N_curves = recording.numColums();

		for (size_t crvIdx = 1; crvIdx < N_curves; ++crvIdx)
		{
			m_dataModel->SetColumnColor(crvIdx, dlg.GetCurveColor(crvIdx));
			m_Plotter->SetCurveVisible (crvIdx, dlg.GetCurveVisible(crvIdx));
			m_Table->SetColumnVisible  (crvIdx, dlg.GetColumnVisible(crvIdx));
		}
		//MainWnd::kGraphBkgrColor = dlg.GetBackgroundColor();
		//MainWnd::kGraphFrameColor = dlg.GetFrameColor();
		//MainWnd::kGraphGridColor = dlg.GetGridlineColor();


  //      m_Plotter->SetShowTitle(MeasurementRecordingPlotter::kShowTitle = dlg.GetShowTitle());
		//m_Plotter->SetShowSubTitle(MeasurementRecordingPlotter::kShowSubTitle = dlg.GetShowSubTitle());

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
