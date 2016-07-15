#include "RecordingTableView.h"

#include "DefaultValues.h"
#include "Recording.h"
#include "RecordingDataModel.h"

#include <QHeaderView>
#include <QScrollBar>

// static
DefaultBools RecordingTableView::sColumnVisible{ true };

//------------------------------------------------------------------------------
RecordingTableView::RecordingTableView(const std::shared_ptr<RecordingDataModel>& model)
    : m_model(model)
{
    ReadSettings();

	const Recording& recording = model->GetRecording();
	const size_t numColumns = recording.numColums();

    // Initialize curve visibility from the defaults, static sColumnVisible, but 
    // only for existing columns.
	m_columnVisible = std::vector<bool>(numColumns, true);
    for (size_t ci = 0;  ci < numColumns;  ++ci) {
        m_columnVisible.at(ci) = sColumnVisible.defaultInRecording(recording, ci);
    }
	
    setModel(model.get());
    setAttribute(Qt::WA_DeleteOnClose);

    verticalHeader()->hide();
    setAlternatingRowColors(true);
    setStyleSheet("selection-background-color: lightblue");
    setStyleSheet("QHeaderView::section { background-color:lightgray }");

    UpdateColumnVisibility();
}
//------------------------------------------------------------------------------
RecordingTableView::~RecordingTableView()
{
    WriteSettings();
}
//------------------------------------------------------------------------------

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
		setColumnHidden(static_cast<int>(columnIdx),  !visible);
        sColumnVisible.setDefaultInRecording(m_model->GetRecording(), columnIdx, visible);
    } else {
        assert(!"Invalid curve index");
    }
}

void RecordingTableView::UpdateColumnVisibility()
{
    const Recording& recording = m_model->GetRecording();
    const size_t  numColumns = recording.numColums();
    for (size_t columnIdx = 1; columnIdx < numColumns; ++columnIdx) {
        setColumnHidden(static_cast<int>(columnIdx), !sColumnVisible.defaultInRecording(recording, columnIdx));
    }
}

//static
void RecordingTableView::ReadSettings()
{
    static bool alreadyLoaded = false;
    if (!alreadyLoaded) {
        sColumnVisible.loadSettings("columnsVisible");
        alreadyLoaded = true;
    }
}

//static
void RecordingTableView::WriteSettings()
{
    sColumnVisible.saveSettings("columnsVisible");
}
