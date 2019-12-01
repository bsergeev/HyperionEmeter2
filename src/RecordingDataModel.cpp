#include "RecordingDataModel.h"

#include "HypReader.h"
#include "Recording.h"
#include "RecordingTableView.h"

DefaultColors RecordingDataModel::sCurveColor{Qt::black};
//{ std::initializer_list<QColor>{
//        QColor(Qt::black),
//        QColor(Qt::blue),
//        QColor(Qt::red),
//        QColor(Qt::darkRed),
//        QColor(Qt::darkCyan),
//        QColor(Qt::magenta),
//        QColor(Qt::darkYellow),
//        QColor(Qt::darkGreen),
//        QColor(Qt::green),
//        QColor(Qt::darkBlue),
//        QColor(Qt::cyan),
//        QColor(Qt::darkMagenta),
//        QColor(Qt::yellow),
//        QColor(Qt::darkBlue),
//        QColor(Qt::darkCyan),
//        QColor(Qt::darkGray)}};

RecordingDataModel::RecordingDataModel(const Recording& rec, QObject* parent)
    : QAbstractTableModel(parent)
    , m_recording(rec)
{
    m_curveColor = sCurveColor.GetValues();
}
//------------------------------------------------------------------------------
QString RecordingDataModel::GetRecordingTitle()    const { 
    return QString::fromStdString(m_recording.GetTitle()); 
}
QString RecordingDataModel::GetRecordingSubTitle() const { 
    return QString::fromStdString(m_recording.GetSubtitle()); 
}
QColor  RecordingDataModel::GetColumnColor(size_t columnIndex) const {
    return m_curveColor.at(columnIndex % m_curveColor.size());
}
void RecordingDataModel::SetColumnColor(size_t columnIndex, const QColor& clr) {
    const size_t i = columnIndex % m_curveColor.size();
    sCurveColor.at(i) = m_curveColor.at(i) = clr;
}
//------------------------------------------------------------------------------
int RecordingDataModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_recording.size());
}
//------------------------------------------------------------------------------
int RecordingDataModel::columnCount(const QModelIndex&) const
{
    return static_cast<int>(m_recording.numColums());
}
//------------------------------------------------------------------------------
QVariant RecordingDataModel::data(const QModelIndex& index, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter | Qt::AlignVCenter);

    case Qt::ForegroundRole:
        if (RecordingTableView::sColorColumns)
            return QVariant(sCurveColor.at(index.column()));
        else
            return QVariant((int)Qt::black);

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
#if 1                                         
        const size_t row = static_cast<size_t>(index.row());
        const size_t col = static_cast<size_t>(index.column());
        return QVariant(QString::number(m_recording.GetValue(row, ColumnIdx(col)), 'f', 
                                        static_cast<int>(m_recording.SeriesPrecision(ColumnIdx(col)))));
#else
        Measurement const& m = thisFlight[index.row()];
        double t = m[kIdxTime];
        double v = m[index.column()];

        double roundFactor = 100.0;
        switch (index.column()) {
        case kIdxTime:   roundFactor = (thisFlight.GetSamplingRate() >= 1.0) ? 1.0 : 10.0;
            break;
        case kIdxThrust: roundFactor = (thisFlight.GetIsBritishUnits()) ? 100.0 : 1.0;
            break;
        case kIdxEfficiency: roundFactor = 1.0;
            break;
        }
        v = floor(roundFactor * v + 0.5) / roundFactor;

        if (role == Qt::DisplayRole) {
            if (index.column() == kIdxEfficiency  &&  v > 100.0)
                return QVariant();
            else
                return QVariant(v);
        }
        else // role == Qt::ToolTipRole
        {
            QString toolTip = tr("Time: ") + MainWnd::GetTimeTxt(t) + QChar('\n')
                + WattmeterDataMgr::GetCurveTemplate(crvIdx).arg(v);
            return QVariant(toolTip);
        }
#endif
    }
    }

    return QVariant();
}
//------------------------------------------------------------------------------
QVariant RecordingDataModel::headerData(int col, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal
         && 0 <= col && col < static_cast<int>(m_recording.numColums()))
        {
            QString title(m_recording.SeriesName(ColumnIdx(col)));
            title.replace(", ", "\n");
            return title;
        }
    }
    return QVariant::Invalid;
}
//------------------------------------------------------------------------------
//static
void RecordingDataModel::ReadSettings()
{
    static bool alreadyLoaded = false;
    if (!alreadyLoaded)
    {
        sCurveColor.at( 0) = QColor(Qt::black);
        sCurveColor.at( 1) = QColor(Qt::blue);
        sCurveColor.at( 2) = QColor(Qt::red);
        sCurveColor.at( 3) = QColor(Qt::darkRed);
        sCurveColor.at( 4) = QColor(Qt::darkCyan);
        sCurveColor.at( 5) = QColor(Qt::magenta);
        sCurveColor.at( 6) = QColor(Qt::darkYellow);
        sCurveColor.at( 7) = QColor(Qt::darkGreen);
        sCurveColor.at( 8) = QColor(Qt::green);
        sCurveColor.at( 9) = QColor(Qt::darkBlue);
        sCurveColor.at(10) = QColor(Qt::cyan);
        sCurveColor.at(11) = QColor(Qt::darkMagenta);
        sCurveColor.at(12) = QColor(Qt::yellow);
        sCurveColor.at(13) = QColor(Qt::darkBlue);
        sCurveColor.at(14) = QColor(Qt::darkCyan);
        sCurveColor.at(15) = QColor(Qt::darkGray);

        sCurveColor.loadSettings("curveColors");

        alreadyLoaded = true;
    }
}

//static
void RecordingDataModel::WriteSettings()
{
    sCurveColor.saveSettings("curveColors");
}
