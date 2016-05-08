#include "SamplePoint.h"

#include <iomanip> // std::setprecision

SamplePoint::SamplePoint(const gsl::span<uint8_t, HypReader::RECORD_LENGTH>& data)
{
    assert(data.data() != nullptr);

    auto twoUChars2Int = [](uint8_t hi, uint8_t lo) {
        return static_cast<int16_t>((hi << 8) | lo);
    };

    volts    = twoUChars2Int(data[0],  data[ 1]) / 100.0;
    amps     = twoUChars2Int(data[2],  data[ 3]) / 100.0;
    mAhOut   = twoUChars2Int(data[4],  data[ 5]);
    mAhIn    = twoUChars2Int(data[6],  data[ 7]);
    RPM      = twoUChars2Int(data[8],  data[ 9]);
    seconds  = twoUChars2Int(data[10], data[11]);
    altitude = twoUChars2Int(data[12], data[13]);        // <<< TBD: verify negative?
    temp1    = twoUChars2Int(data[14], data[15]) / 10.0;
    temp2    = twoUChars2Int(data[16], data[17]) / 10.0;
    temp3    = twoUChars2Int(data[18], data[19]) / 10.0;
    throttle = twoUChars2Int(data[20], data[21]);
    tempAmb  = twoUChars2Int(data[22], data[23]) / 10.0; // <<< TBD: verify negative
}

std::ostream& operator <<(std::ostream& os, const SamplePoint& r)
{
    static const char* s = "\t"; // separator
    os << std::fixed << std::setprecision(2) << r.seconds  << s 
       << std::fixed << std::setprecision(2) << r.volts    << s
       << std::fixed << std::setprecision(2) << r.amps     << s
       << std::fixed << std::setprecision(0) << r.mAhOut   << s
       << std::fixed << std::setprecision(0) << r.mAhIn    << s
       << std::fixed << std::setprecision(0) << r.RPM      << s
       << std::fixed << std::setprecision(0) << r.altitude << s
       << std::fixed << std::setprecision(1) << r.temp1    << s
       << std::fixed << std::setprecision(1) << r.temp2    << s
       << std::fixed << std::setprecision(1) << r.temp3    << s
       << std::fixed << std::setprecision(0) << r.throttle << s
       << std::fixed << std::setprecision(1) << r.tempAmb;
    return os;
}
