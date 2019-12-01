#pragma once

#include "Recording.h"
#include "SamplePoint.h"

#include <array>

#include <QColor>
#include <QSettings>
#include <QString>


template <typename T, typename TReader,
          size_t   N>
          //typename std::enable_if<std::is_literal_type<T>::value>::type * = nullptr>
class DefaultValues_t
{
public:
             DefaultValues_t() = default;
    explicit DefaultValues_t(const T& v) noexcept { m_vals.fill(v); }

    const std::array<T, N>& GetValues() const { return m_vals;  }
    T  at(size_t idx) const { return m_vals.at(idx); }
    T& at(size_t idx)       { return m_vals.at(idx); }

    T defaultInRecording(const Recording& rec, size_t colIdx) {
        const SamplePoint::ValueIndex vi = rec.GetColumnType(ColumnIdx{ colIdx });
        if (vi < N) {
            return m_vals.at(vi);
        } else {
            assert(!"Invalid value index");
            return static_cast<T>(0);
        }
    }

    void setDefaultInRecording(const Recording& rec, size_t colIdx, T v) {
        const SamplePoint::ValueIndex vi = rec.GetColumnType(ColumnIdx{ colIdx });
        if (vi < N) {
            m_vals.at(vi) = v;
        } else {
            assert(!"Invalid value index");
        }
    }

    void saveSettings(const QString& name) {
        QSettings settings;
        settings.beginWriteArray(name, N);
        for (int i = 0; i < N; ++i) {
            settings.setArrayIndex(i);
            settings.setValue("v", m_vals.at(i));
        }
        settings.endArray();
    }

    void loadSettings(const QString& name) {
        QSettings settings;
        const int size = settings.beginReadArray(name);
        if (TReader* const reader = static_cast<TReader*>(this)) {
            for (int i = 0; i < size; ++i) {
                settings.setArrayIndex(i);
                m_vals.at(i) = reader->loadSetting(settings, "v");
            }
        }
        settings.endArray();
    }

protected:
    std::array<T, N> m_vals; // indexed by SamplePoint::ValueIndex, i.e. for all curves/columns
}; // class DefaultValues_t
//------------------------------------------------------------------------------

// bool "specialization" of DefaultValues_t
template <size_t N>
class DefaultBools_t : public DefaultValues_t<bool, DefaultBools_t<N>, N>
{
public:
    DefaultBools_t(bool v) noexcept : DefaultValues_t(v) {}

    bool loadSetting(QSettings& settings, const QString& name) {
        return settings.value(name).toBool();
    }
};
typedef DefaultBools_t<SamplePoint::eNUM_VALUES> DefaultBools;


// QColor "specialization" of DefaultValues_t
template <size_t N>
class DefaultColor_t : public DefaultValues_t<QColor, DefaultColor_t<N>, N>
{
public:
    DefaultColor_t(QColor v) noexcept : DefaultValues_t(v) {}

    QString loadSetting(QSettings& settings, const QString& name) {
        return settings.value(name).toString();
    }
};
typedef DefaultColor_t<SamplePoint::eNUM_VALUES> DefaultColors;
