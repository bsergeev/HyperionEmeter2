#include "RecordingDataModel.h"

#include "HypReader.h"
#include "Recording.h"

RecordingDataModel::RecordingDataModel(HypReader* reader, size_t recIdx)
    : m_recording(reader->GetRecording(recIdx))
{
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
        //if (m_PlotSettingsMgr) {
        //    return (crvIdx == kIdxTime || !m_PlotSettingsMgr->GetColorColumns()) ? Qt::black
        //        : m_PlotSettingsMgr->GetCurveColor(crvIdx);
        //}
        //else
            return QVariant((int)Qt::black);

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
#if 1                                         
        const size_t row = static_cast<size_t>(index.row());
        const size_t col = static_cast<size_t>(index.column());
        return QVariant(m_recording.GetValue(row, col));
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
         && 0 <= col && col < m_recording.numColums())
        {
            return QString(m_recording.SeriesName(col));
        }
    }
    return QVariant::Invalid;
}
//------------------------------------------------------------------------------
