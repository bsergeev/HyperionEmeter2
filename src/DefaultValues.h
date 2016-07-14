#pragma once

#include "SamplePoint.h"

#include <array>

class QString;
class Recording;

class DefaultValues
{
public:
             DefaultValues()       noexcept = default;
    explicit DefaultValues(bool v) noexcept { m_vals.fill(v); }

    bool  at(size_t idx) const { return m_vals.at(idx); }
    bool& at(size_t idx)       { return m_vals.at(idx); }

    bool    defaultInRecording(const Recording& rec, size_t colIdx);
    void setDefaultInRecording(const Recording& rec, size_t colIdx, bool v);

    void saveSettings(const QString& name);
    void loadSettings(const QString& name);

private:
    std::array<bool, SamplePoint::eNUM_VALUES> m_vals; // indexed by SamplePoint::ValueIndex, i.e. for all curves/columns
};
