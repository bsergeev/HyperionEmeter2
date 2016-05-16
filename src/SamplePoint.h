#pragma once

#include <iosfwd>
#include <stdint.h>
#include <type_traits>

#include <span.h>        // gsl::span
#include "HypCommands.h" // RECORD_LENGTH

// Utility constexpr function converting a enum class to its underlying type
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
        eSeconds  = 0,
        eVolts    = 1,
        eAmps     = 2,
        emAh_Out  = 3,
        emAh_In   = 4,
        eRPM      = 5,
        eAltitude = 6,
        eTemp1    = 7,
        eTemp2    = 8,
        eTemp3    = 9,
        eThrottle = 10,
        eTempAmb  = 11,
        eNUM_VALUES // the last
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
