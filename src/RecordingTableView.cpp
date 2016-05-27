#include "RecordingTableView.h"
#include "RecordingDataModel.h"

#include <QScrollBar>
#include <QHeaderView>

RecordingTableView::RecordingTableView(const std::shared_ptr<RecordingDataModel>& model)
    : m_model(model)
{
      setModel(model.get());
      setAttribute(Qt::WA_DeleteOnClose);
}

int RecordingTableView::GetTotalWidth() const
{
    int w = style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2;
    for (int column = 0; column < m_model->columnCount(); ++column) {
        w += columnWidth(column);
    }
    return w;
}