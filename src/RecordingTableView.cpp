#include "RecordingTableView.h"

#include "DefaultValues.h"
#include "Recording.h"
#include "RecordingDataModel.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QSettings>

// static
DefaultValues RecordingTableView::sColumnVisible{ true };

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
		setColumnHidden   (columnIdx,  !visible);
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
        setColumnHidden(columnIdx, !sColumnVisible.defaultInRecording(recording, columnIdx));
    }
}

//static
void RecordingTableView::ReadSettings()
{
    static bool alreadyLoaded = false;
    if (!alreadyLoaded)
    {
        QSettings settings;
        const int size = settings.beginReadArray("columnsVisible");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            sColumnVisible.at(i) = settings.value("visible").toBool();
        }
        settings.endArray();
    }
}

//static
void RecordingTableView::WriteSettings()
{
    QSettings settings;
    settings.beginWriteArray("columnsVisible", SamplePoint::eNUM_VALUES);
    for (int i = 0; i < SamplePoint::eNUM_VALUES; ++i) {
        settings.setArrayIndex(i);
        settings.setValue("visible", sColumnVisible.at(i));
    }
    settings.endArray();
}
