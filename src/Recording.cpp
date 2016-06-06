#include "Recording.h"
#include "SamplePoint.h"

#include <iomanip> // std::setprecision

//------------------------------------------------------------------------------
Recording::Recording(std::vector<SamplePoint>&& points)
    : m_points(points)
{
    m_hasData.fill(false);  
    m_hasData[SamplePoint::eSeconds] = true; // time is always present
    
    // Set default column to SamplePoint index mapping
    for (size_t i = 0; i < SamplePoint::eNUM_VALUES; ++i) {
        m_column2ptIdx[i] = i;
        m_pt2columnIdx[i] = ColumnIdx{ i };
    }
}
//------------------------------------------------------------------------------
// return whether anything was changed
bool Recording::MassageData()
{
    const size_t N_points = m_points.size();
    if (N_points == 0) {
        return false; // nothing to do
    }

    bool changed = false;

    // First, check if the sample rate is less than a second, thus several
    // equal 'seconds' in a row should be fixed.
    size_t max_same_seconds_inarow = 1;
    {
        size_t same_seconds_inarow = 1;
        double prev_seconds = -1.0; // not set
        for (const SamplePoint& pt : m_points) {
            const double& seconds = pt[SamplePoint::eSeconds];
            if (prev_seconds >= 0.0) {
                if (prev_seconds == seconds) {
                    ++same_seconds_inarow;
                    if (max_same_seconds_inarow < same_seconds_inarow) {
                        max_same_seconds_inarow = same_seconds_inarow;
                    }
                } else {
                    prev_seconds = seconds;
                    same_seconds_inarow = 1;
                }
            } else { // it's 1st pass
                prev_seconds = seconds;
            }
        }
    }
    if (max_same_seconds_inarow > 1) 
    {
        // Occasionally, very short sequences don't have a full sub-sequence
        if (max_same_seconds_inarow == 3 && N_points < 7) {
            max_same_seconds_inarow = 4;
        }
        assert(max_same_seconds_inarow == 2 || max_same_seconds_inarow == 4);
        const double ds = 1.0 / max_same_seconds_inarow;

        double prev_seconds = -1.0; // not set
        size_t prev_sec_idx = 0;
        for (size_t i = 0; i < N_points; ++i) {
            const SamplePoint& pt = m_points[i];
            const double& seconds = pt[SamplePoint::eSeconds];
            if (prev_seconds >= 0.0) {
                if (prev_seconds != seconds) { // value just changed
                    if (prev_sec_idx + 1 < i) {
                        changed = true;
                        const double s0 = seconds - (i - prev_sec_idx)*ds;
                        for (size_t j = prev_sec_idx; j < i; ++j) {
                            m_points[j][SamplePoint::eSeconds] = s0 + ds*(j - prev_sec_idx);
                        }
                    }
                    prev_seconds = seconds;
                    prev_sec_idx = i;
                }
            } else { // it's 1st pass
                prev_seconds = seconds;
            }
        }
        if (prev_sec_idx + 1 < N_points) {
            changed = true;
            const double s0 = m_points[prev_sec_idx][SamplePoint::eSeconds];
            for (size_t j = prev_sec_idx+1; j < N_points; ++j) {
                m_points[j][SamplePoint::eSeconds] = s0 + ds*(j - prev_sec_idx);
            }
        }
    }

    // Next, record more accurate discharge values
    const SamplePoint& pt0 = m_points.front();
    double discharged = pt0[SamplePoint::emAh_Out];
    double prev_amps  = pt0[SamplePoint::eAmps];
    const double dt = (N_points > 1)? m_points[1][SamplePoint::eSeconds] - pt0[SamplePoint::eSeconds]
                                    : 0.0;
    for (size_t i = 1; i < N_points; ++i) {
        SamplePoint& pt = m_points[i];
        const double orig_discharge = pt[SamplePoint::emAh_Out];
        const double& amps = pt[SamplePoint::eAmps];
        pt[SamplePoint::emAh_Out] = discharged += 0.5*(prev_amps + amps)*dt;
        changed |= (orig_discharge != pt[SamplePoint::emAh_Out]);
        prev_amps = amps;
    }

    // Find which series have (non-const) data
    m_pt2columnIdx.fill(ColumnIdx{ 0 });
    m_numHasData = 1;
    for (size_t j = 1; j < SamplePoint::eNUM_VALUES; ++j) {
        m_hasData[j] = false;
        for (size_t i = 1; i < N_points; ++i) {
            const SamplePoint& pt = m_points[i];
            if (fabs(pt[j] - pt0[j]) > 0.001) { // as the max accuracy is 0.01
                m_hasData[j] = true;
                m_pt2columnIdx[j] = ColumnIdx{ m_numHasData };
                m_column2ptIdx[m_numHasData++] = j;
                break;
            }
        }
    }

    // Record min/max values for all curves
    for (size_t j = 0; j < SamplePoint::eNUM_VALUES; ++j) {
        double minV =  std::numeric_limits<double>::max();
        double maxV = -std::numeric_limits<double>::max();
        for (size_t i = 0; i < N_points; ++i) {
            const double v = m_points[i][j];
            if (minV > v) {    minV = v; }
            if (maxV < v) { maxV = v; }
        }
        assert(minV <= maxV);
        CurveInfo& ci = m_curveInfo[j];
        ci.minV = minV;
        ci.maxV = maxV;
    }


    return changed;
}
//------------------------------------------------------------------------------
void Recording::PrintHeader(std::ostream& os, bool skipEmpty) const
{
    if (m_points.empty()) {
        os << "Empty recording\n";
        return;
    }

    // First, list const series
    if (skipEmpty) {
        const SamplePoint& pt0 = m_points.front();
        bool first = true;
        for (size_t i = 1; i < SamplePoint::eNUM_VALUES; ++i) {
            if (!m_hasData[i]) {
                os << ((first)? "Constant values: ":"; ") << SamplePoint::SeriesName(i) << " = " 
                   << std::fixed<<std::setprecision(SamplePoint::sSeriesPrecision[i]) << pt0[i];
                first = false;
            }
        }
        os << std::endl;
    }

    // Next, columns with varying data
    os << SamplePoint::SeriesName(0); // seconds are always present
    for (size_t i = 1; i < SamplePoint::eNUM_VALUES; ++i) {
        if (!skipEmpty || m_hasData[i]) {
            os << "\t" << SamplePoint::SeriesName(i);
        }
    }
    os << std::endl;
}
//------------------------------------------------------------------------------
void Recording::PrintData(std::ostream& os, bool skipEmpty) const
{
    for (const auto& pt : m_points) {
        os << std::fixed << std::setprecision(SamplePoint::sSeriesPrecision[0]) << pt[0]; // seconds are always present
        for (size_t i = 1; i < SamplePoint::eNUM_VALUES; ++i) {
            if (!skipEmpty || m_hasData[i]) {
                os << "\t" << std::fixed << std::setprecision(SamplePoint::sSeriesPrecision[i]) << pt[i];
            }
        }
        os << std::endl;
    }
}
//------------------------------------------------------------------------------
double Recording::GetValue(size_t row, ColumnIdx col) const
{
    double v = 0.0;
    if (0 <= row && row < m_points.size()
     && 0 <= col && col < numColums()) {
        const SamplePoint& pt = m_points.at(row);
        return pt[m_column2ptIdx[col]];
    }
    return v;
}
//------------------------------------------------------------------------------
const CurveInfo& Recording::GetCurveInfo(ColumnIdx col) const
{
    if (0 <= col && col < numColums()) {
        return m_curveInfo[m_column2ptIdx[col]];
    } else {
        assert(!"Invalid column index");
        const static CurveInfo error;
        return error;
    }
}
//------------------------------------------------------------------------------
const char* Recording::SeriesName(ColumnIdx col) const
{
    const char* name = nullptr;
    if (0 <= col && col < numColums()) { 
        name = SamplePoint::SeriesName(m_column2ptIdx[col]);
    }
    return name;
}
//------------------------------------------------------------------------------
size_t Recording::SeriesPrecision(ColumnIdx col) const
{
    size_t precision = 0;
    if (0 <= col && col < numColums()) { 
        precision = SamplePoint::sSeriesPrecision[m_column2ptIdx[col]];
    }
    return precision;
}
//------------------------------------------------------------------------------
SamplePoint::ValueIndex Recording::GetColumnType(ColumnIdx col) const
{
    assert(col < SamplePoint::eNUM_VALUES && "Invalid column");
    return static_cast<SamplePoint::ValueIndex>(m_column2ptIdx[col]);
}
//------------------------------------------------------------------------------
ColumnIdx Recording::GetColumnOfType(SamplePoint::ValueIndex t) const
{
    assert(0 <= t && t < SamplePoint::eNUM_VALUES && "Invalid type");
    return m_pt2columnIdx[t];
}
//------------------------------------------------------------------------------
bool Recording::HasDataOfType(SamplePoint::ValueIndex t) const
{
    assert(0 <= t && t < SamplePoint::eNUM_VALUES && "Invalid type");
    return m_hasData[t];
}
//------------------------------------------------------------------------------
void Recording::CalculatePropDependentValues(std::function<PowerOutAndThrust(double)> calcCBk)
{
    assert(m_hasData[SamplePoint::eRPM] && "Can only calculate, when RPM was measured");
    for (auto& pt : m_points) {
        const PowerOutAndThrust pw_th = calcCBk(pt[SamplePoint::eRPM]);
        pt[SamplePoint::ePowerOut] = pw_th.powerOut;
        pt[SamplePoint::eThrust  ] = pw_th.thrust;
    }
}
//------------------------------------------------------------------------------