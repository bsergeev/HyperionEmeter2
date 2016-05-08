#pragma once

#include <stdint.h>
#include <vector>

class SamplePoint;

class Recording 
{
    friend std::ostream& operator <<(std::ostream& os, const Recording& obj);
public:
    Recording(std::vector<SamplePoint>&& points);

    static void PrintHeader (std::ostream& os);
    static void PrintDivider(std::ostream& os);

private:
    std::vector <SamplePoint> m_points;
};

std::ostream& operator <<(std::ostream& os, const Recording& obj);
