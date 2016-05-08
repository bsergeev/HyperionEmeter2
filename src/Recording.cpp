#include "Recording.h"
#include "SamplePoint.h"

Recording::Recording(std::vector<SamplePoint>&& points)
    : m_points(points)
{
}

//static
void Recording::PrintHeader(std::ostream& os)
{
    PrintDivider(os);
    os << "Seconds\tVoltage, V\tCurrent, A\tDisch, mAh\tCharge, mAh\tRPM\tAltitude, m\tTemp1, C\tTemp2, C\tTemp3, C\tThrottle, uS\tAmb.Temp, C\n";
}

//static
void Recording::PrintDivider(std::ostream& os)
{
    os << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
}

std::ostream& operator <<(std::ostream& os, const Recording& r)
{
    for (auto pt : r.m_points) {
        os << pt << "\n";
    }
    return os;
}
