#pragma once

#include "SamplePoint.h"

#include <array>
#include <iosfwd>
#include <vector>

class Recording 
{
public:
    Recording(std::vector<SamplePoint>&& points);

    bool MassageData(); // return whether anything was changed

    void PrintHeader(std::ostream& os, bool skipEmpty = true) const;
    void PrintData  (std::ostream& os, bool skipEmpty = true) const;

private:
    std::vector<SamplePoint> m_points;
    std::array<bool, ut(SamplePoint::ValueIndex::NUM_VALUES)> m_hasData;
};
