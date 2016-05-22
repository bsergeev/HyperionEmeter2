#include "RecordingPlotter.h"
#include "RecordingDataModel.h"

#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QColor>
#include <QAction>
#include <QToolTip>

//------------------------------------------------------------------------------

const short RecordingPlotter::GAP_X = 4;
const int   RecordingPlotter::NO_STEPS_PROCESSED = -1;
bool        RecordingPlotter::kShowTooltip  = true;
bool        RecordingPlotter::kShowTitle    = true;
bool        RecordingPlotter::kShowSubTitle = true;

//------------------------------------------------------------------------------

RecordingPlotter::RecordingPlotter(const std::shared_ptr<RecordingDataModel>& model,
    QWidget* parent)
    : QAbstractItemView(parent)
    , m_model(model)
    , m_ParentWnd(parent)
{
}
//------------------------------------------------------------------------------
RecordingPlotter::~RecordingPlotter()
{
}

//------------------------------------------------------------------------------
QModelIndex RecordingPlotter::indexAt(const QPoint &p) const
{
    return QModelIndex();
}

//------------------------------------------------------------------------------

