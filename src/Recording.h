#pragma once

#include "SamplePoint.h"

#include <array>
#include <iosfwd>
#include <vector>

class Recording 
{
public:
    explicit Recording(std::vector<SamplePoint>&& points);

    bool MassageData(); // return whether anything was changed

    void PrintHeader(std::ostream& os, bool skipEmpty = true) const;
    void PrintData  (std::ostream& os, bool skipEmpty = true) const;

    size_t size()      const { return m_points.size(); }
    size_t numColums() const { return m_numHasData;  }
    double GetValue(size_t row, size_t col) const;

    const char* SeriesName(size_t col) const;
    size_t SeriesPrecision(size_t col) const;

private:
    std::vector<SamplePoint> m_points;

    std::array        <bool,   SamplePoint::eNUM_VALUES> m_hasData;
    mutable std::array<size_t, SamplePoint::eNUM_VALUES> m_column2ptIdx;
    mutable size_t  m_numHasData = 1; // time is always there
};
