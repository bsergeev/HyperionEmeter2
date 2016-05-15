#pragma once

#include <iosfwd>
#include <vector>

class SamplePoint;

class Recording 
{
    friend std::ostream& operator <<(std::ostream& os, const Recording& obj);
public:
    Recording(std::vector<SamplePoint>&& points);

    bool MassageData(); // return whether anything was changed

    static void PrintHeader (std::ostream& os);
    static void PrintDivider(std::ostream& os);

private:
    std::vector<SamplePoint> m_points;
};

std::ostream& operator <<(std::ostream& os, const Recording& obj);
