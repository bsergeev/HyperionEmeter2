#pragma once

#include <stdint.h>

#include <span.h>        // gsl::span
#include "HypCommands.h" // RECORD_LENGTH

// Utility constexpr function converting a enum class to its underlying type N/U
#include <type_traits>
template <typename E,
          typename = std::enable_if_t<std::is_enum<E>::value>>
constexpr auto ut(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}


class SamplePoint {
public:
    explicit SamplePoint(const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& data);

    enum ValueIndex {
        eSeconds = 0,
        eVolts   ,
        eAmps    ,
        emAh_Out , // recalculated
        emAh_In  , // -"- (is rarely present)
        eRPM     , // []
        eAltitude, //  RDU
        eTemp1   , // []
        eTemp2   , // []
        eTemp3   , // []
        eThrottle, // [RDU]
        eTempAmb , //  RDU

        ePowerIn,
        ePowerOut, // [PRM]
        eEfficiency,//[PRM]
        eThrust,   // [RPM] 
        eNUM_VALUES
    };

    inline double& operator [](size_t idx) {
        if (idx < eNUM_VALUES) {
            return m_values[idx];
        } else {
            assert(!"Invalid index");
            static double ERROR = 0.0;
            return ERROR;
        }
    }

    inline const double& operator [](size_t idx) const {
        if (idx < eNUM_VALUES) {
            return m_values[idx];
        } else {
            assert(!"Invalid index");
            static const double ERROR = 0.0;
            return ERROR;
        }
    }

//static:
    static const char* SeriesName(size_t idx) {
        if (idx < eNUM_VALUES) { return sSeriesNames[idx]; } 
        else                   { return "ERROR"; }
    }

    static const std::array<size_t, eNUM_VALUES> sSeriesPrecision;

private:
    std::array<double, eNUM_VALUES> m_values;
//static
    static const std::array<const char*, eNUM_VALUES> sSeriesNames;
};
