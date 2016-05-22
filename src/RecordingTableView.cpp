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
