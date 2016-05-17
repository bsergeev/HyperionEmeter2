#pragma once

#include <QAbstractItemModel>

class HypReader;
class Recording;

class RecordingDataModel : public QAbstractTableModel
{
public:
    RecordingDataModel(HypReader* reader, size_t recIdx);

    virtual int      rowCount   (const QModelIndex& parent = QModelIndex()) const override;
    virtual int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data       (const QModelIndex& index, int role = Qt::DisplayRole) const  override;
    virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const override;

private:
    const Recording& m_recording;
};
