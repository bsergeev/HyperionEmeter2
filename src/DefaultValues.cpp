#include "DefaultValues.h"
#include "Recording.h"

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
