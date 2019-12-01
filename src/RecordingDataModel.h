#pragma once

#include "DefaultValues.h"

#include <QAbstractItemModel>
#include <QString>
#include <QColor>

#include <array>

class Recording;

class RecordingDataModel : public QAbstractTableModel
{
public:
    explicit RecordingDataModel(const Recording& rec, QObject* parent = nullptr);

    virtual int      rowCount   (const QModelIndex& parent = QModelIndex()) const override;
    virtual int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data       (const QModelIndex& i, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData (int section, Qt::Orientation orient, int role) const override;

    const Recording& GetRecording() const { return m_recording; }
    QString GetRecordingTitle()     const;
    QString GetRecordingSubTitle()  const;

    QColor GetColumnColor(size_t columnIndex) const;
    void   SetColumnColor(size_t columnIndex, const QColor& clr);

//static:
    static void ReadSettings();
    static void WriteSettings();

private:
    const Recording& m_recording;
    std::array<QColor, SamplePoint::eNUM_VALUES> m_curveColor;

    static DefaultColors sCurveColor;
};
