#include "DefaultValues.h"

#include "Recording.h"

#include <QSettings>
#include <QString>

bool DefaultValues::defaultInRecording(const Recording& rec, size_t colIdx)
{
    const SamplePoint::ValueIndex vi = rec.GetColumnType(ColumnIdx{ colIdx });
    if (vi < SamplePoint::eNUM_VALUES) {
        return m_vals[vi];
    } else {
        assert(!"Invalid value index");
        return false;
    }
}

void DefaultValues::setDefaultInRecording(const Recording& rec, size_t colIdx, bool v)
{
    const SamplePoint::ValueIndex vi = rec.GetColumnType(ColumnIdx{ colIdx });
    if (vi < SamplePoint::eNUM_VALUES) {
        m_vals[vi] = v;
    } else {
        assert(!"Invalid value index");
    }
}

void DefaultValues::saveSettings(const QString& name)
{
    QSettings settings;
    settings.beginWriteArray(name, SamplePoint::eNUM_VALUES);
    for (int i = 0; i < SamplePoint::eNUM_VALUES; ++i) {
        settings.setArrayIndex(i);
        settings.setValue("visible", m_vals.at(i));
    }
    settings.endArray();
}

void DefaultValues::loadSettings(const QString& name)
{
    QSettings settings;
    const int size = settings.beginReadArray(name);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        m_vals.at(i) = settings.value("visible").toBool();
    }
    settings.endArray();
}

