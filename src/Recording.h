#pragma once

#include "SamplePoint.h"
#include "TypeWrapper.h"

#include <array>
#include <iosfwd>
#include <limits>
#include <vector>

struct CurveInfo {
	double minV = std::numeric_limits<double>::max();
	double maxV = std::numeric_limits<double>::min();
};

class Recording 
{
public:
    explicit Recording(std::vector<SamplePoint>&& points);

    bool MassageData(); // return whether anything was changed

    void PrintHeader(std::ostream& os, bool skipEmpty = true) const;
    void PrintData  (std::ostream& os, bool skipEmpty = true) const;

    size_t size()      const { return m_points.size(); }
    size_t numColums() const { return m_numHasData;  }

	double   GetValue(size_t row, ColumnIdx col) const;
	const CurveInfo& GetCurveInfo(ColumnIdx col) const;
	const char* SeriesName(ColumnIdx col) const;
    size_t SeriesPrecision(ColumnIdx col) const;

	SamplePoint::ValueIndex GetColumnType(ColumnIdx col) const;
	ColumnIdx GetColumnOfType(SamplePoint::ValueIndex t) const;

private:
    std::vector<SamplePoint> m_points;
	std::array<CurveInfo, SamplePoint::eNUM_VALUES> m_curveInfo;

    size_t m_numHasData = 1; // 1 since Seconds is always there
    std::array<bool,      SamplePoint::eNUM_VALUES> m_hasData;
    std::array<size_t,    SamplePoint::eNUM_VALUES> m_column2ptIdx; // first m_numHasData elements are used
	std::array<ColumnIdx, SamplePoint::eNUM_VALUES> m_pt2columnIdx; // values < m_numHasData
};
