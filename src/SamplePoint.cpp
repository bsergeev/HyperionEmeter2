#include "SamplePoint.h"
#include "HypReader.h"

#include <iomanip> // std::setprecision

SamplePoint::SamplePoint(const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& data)
{
    assert(data.data() != nullptr);

    auto twoUChars2Int = [](uint8_t hi, uint8_t lo) {
        return static_cast<int16_t>((hi << 8) | lo);
    };

    m_values[ut(ValueIndex::volts)   ] = twoUChars2Int(data[0],  data[ 1]) / 100.0;
    m_values[ut(ValueIndex::amps)    ] = twoUChars2Int(data[2],  data[ 3]) / 100.0;
    m_values[ut(ValueIndex::mAh_Out) ] = twoUChars2Int(data[4],  data[ 5]);
    m_values[ut(ValueIndex::mAh_In)  ] = twoUChars2Int(data[6],  data[ 7]);
    m_values[ut(ValueIndex::RPM)     ] = twoUChars2Int(data[8],  data[ 9]);
    m_values[ut(ValueIndex::seconds) ] = twoUChars2Int(data[10], data[11]);
    m_values[ut(ValueIndex::altitude)] = twoUChars2Int(data[12], data[13]);        // TODO: verify negative?
    m_values[ut(ValueIndex::temp1)   ] = twoUChars2Int(data[14], data[15]) / 10.0;
    m_values[ut(ValueIndex::temp2)   ] = twoUChars2Int(data[16], data[17]) / 10.0;
    m_values[ut(ValueIndex::temp3)   ] = twoUChars2Int(data[18], data[19]) / 10.0;
    m_values[ut(ValueIndex::throttle)] = twoUChars2Int(data[20], data[21]);
    m_values[ut(ValueIndex::tempAmb) ] = twoUChars2Int(data[22], data[23]) / 10.0; // TODO: verify negative
}

std::ostream& operator <<(std::ostream& os, const SamplePoint& r)
{
    static const char* s = "\t"; // separator
    using vi = SamplePoint::ValueIndex;
    os << std::fixed << std::setprecision(2) << r[vi::seconds ] << s
       << std::fixed << std::setprecision(2) << r[vi::volts   ] << s
       << std::fixed << std::setprecision(2) << r[vi::amps    ] << s
       << std::fixed << std::setprecision(2) << r[vi::mAh_Out ] << s
       << std::fixed << std::setprecision(0) << r[vi::mAh_In  ] << s
       << std::fixed << std::setprecision(0) << r[vi::RPM     ] << s
       << std::fixed << std::setprecision(0) << r[vi::altitude] << s
       << std::fixed << std::setprecision(1) << r[vi::temp2   ] << s
       << std::fixed << std::setprecision(1) << r[vi::temp3   ] << s
       << std::fixed << std::setprecision(1) << r[vi::temp1   ] << s
       << std::fixed << std::setprecision(0) << r[vi::throttle] << s
       << std::fixed << std::setprecision(1) << r[vi::tempAmb ];
    return os;
}
