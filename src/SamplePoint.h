#pragma once

#include <stdint.h>
#include <type_traits>

#include <span.h>        // gsl::span
#include "HypCommands.h" // RECORD_LENGTH

// Utility constexpr function coverting a enum class to its underlying type
template <typename E,
          typename = std::enable_if_t<std::is_enum<E>::value>>
constexpr auto ut(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}


class SamplePoint {
    friend std::ostream& operator <<(std::ostream& os, const SamplePoint& obj);
public:
    SamplePoint(const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& data);

    enum class ValueIndex : size_t {
        seconds  = 0,
        volts    = 1,
        amps     = 2,
        mAh_Out  = 3,
        mAh_In   = 4,
        RPM      = 5,
        altitude = 6,
        temp1    = 7,
        temp2    = 8,
        temp3    = 9,
        throttle = 10,
        tempAmb  = 11,
        NUM_VALUES // the last
    };

    inline double& operator [](ValueIndex idx) {
        if (idx < ValueIndex::NUM_VALUES) {
            return m_values[ut(idx)];
        } else {
            assert(!"Invalid index");
            static double ERROR = 0.0;
            return ERROR;
        }
    }

    inline const double& operator [](ValueIndex idx) const {
        if (idx < ValueIndex::NUM_VALUES) {
            return m_values[ut(idx)];
        } else {
            assert(!"Invalid index");
            static const double ERROR = 0.0;
            return ERROR;
        }
    }

private:
    std::array<double, ut(ValueIndex::NUM_VALUES)> m_values;
};

std::ostream& operator <<(std::ostream& os, const SamplePoint& obj);
