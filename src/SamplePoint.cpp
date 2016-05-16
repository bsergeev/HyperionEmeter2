#include "SamplePoint.h"
#include "HypReader.h"

#include <span.h>  // gsl::span

//static
const std::array<size_t, SamplePoint::eNUM_VALUES> SamplePoint::sSeriesPrecision = {
    2, 2, 2, 2, 0, 0, 0, 1, 1, 1, 0, 1
};
const std::array<const char*, SamplePoint::eNUM_VALUES> SamplePoint::sSeriesNames = {
    "Seconds", "Voltage, V", "Current, A", "Discharge, mAh", "Charge, mAh", "RPM", "Altitude, m", "Temp1, C", "Temp2, C", "Temp3, C", "Throttle, uS", "Amb.Temp, C"
};

SamplePoint::SamplePoint(const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& data)
{
    assert(data.data() != nullptr);

    auto twoUChars2Int = [](uint8_t hi, uint8_t lo) {
        return static_cast<int16_t>((hi << 8) | lo);
    };
    m_values[eVolts   ] = twoUChars2Int(data[0],  data[ 1]) / 100.0;
    m_values[eAmps    ] = twoUChars2Int(data[2],  data[ 3]) / 100.0;
    m_values[emAh_Out ] = twoUChars2Int(data[4],  data[ 5]);
    m_values[emAh_In  ] = twoUChars2Int(data[6],  data[ 7]);
    m_values[eRPM     ] = twoUChars2Int(data[8],  data[ 9]);
    m_values[eSeconds ] = twoUChars2Int(data[10], data[11]);
    m_values[eAltitude] = twoUChars2Int(data[12], data[13]);        // TODO: verify negative?
    m_values[eTemp1   ] = twoUChars2Int(data[14], data[15]) / 10.0;
    m_values[eTemp2   ] = twoUChars2Int(data[16], data[17]) / 10.0;
    m_values[eTemp3   ] = twoUChars2Int(data[18], data[19]) / 10.0;
    m_values[eThrottle] = twoUChars2Int(data[20], data[21]);
    m_values[eTempAmb ] = twoUChars2Int(data[22], data[23]) / 10.0; // TODO: verify negative
}
