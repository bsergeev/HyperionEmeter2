#include "RecordingTableView.h"
#include "RecordingDataModel.h"
#include "Recording.h"

#include <QScrollBar>
#include <QHeaderView>

RecordingTableView::RecordingTableView(const std::shared_ptr<RecordingDataModel>& model)
    : m_model(model)
{
	const Recording& recording = model->GetRecording();
	const size_t N = recording.numColums();
	m_columnVisible = std::vector<bool>(N, true);
	

    setModel(model.get());
    setAttribute(Qt::WA_DeleteOnClose);

    verticalHeader()->hide();
    setAlternatingRowColors(true);
    setStyleSheet("selection-background-color: lightblue");
    setStyleSheet("QHeaderView::section { background-color:lightgray }");
}

int RecordingTableView::GetTotalWidth() const
{
    int w = style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2 + 2; // <<< DEBUG "4" - accounts for frame
    for (int column = 0; column < m_model->columnCount(); ++column) {
        w += columnWidth(column);
    }
    return w;
}

bool RecordingTableView::IsColumnVisible(size_t columnIdx) const
{
    bool visible = false;
    if (columnIdx < m_columnVisible.size()) {
        visible = m_columnVisible.at(columnIdx);
    } else {
        assert(!"Invalid curve index");
    }
    return visible;
}

void RecordingTableView::SetColumnVisible(size_t columnIdx, bool visible)
{
    if (columnIdx < m_columnVisible.size()) {
		m_columnVisible.at(columnIdx) = visible;
		setColumnHidden   (columnIdx,  !visible);
    } else {
        assert(!"Invalid curve index");
    }
}
