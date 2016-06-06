#pragma once

#include <QAbstractItemModel>
#include <QString>
#include <QColor>

#include <array>

class Recording;

class RecordingDataModel : public QAbstractTableModel
{
public:
    explicit RecordingDataModel(const Recording& rec);

    virtual int      rowCount   (const QModelIndex& parent = QModelIndex()) const override;
    virtual int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data       (const QModelIndex& i, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData (int section, Qt::Orientation orient, int role) const override;

    const Recording& GetRecording() const { return m_recording; }
    QString GetRecordingTitle()     const { return "Title"; }
    QString GetRecordingSubTitle()  const { return "Subtitle"; }

	QColor GetColumnColor(size_t columnIndex) const;
	void   SetColumnColor(size_t columnIndex, const QColor& clr);

private:
    const Recording& m_recording;

    static const size_t N_COLORS = 13;
    std::array<QColor, N_COLORS> m_curveColor;
};
