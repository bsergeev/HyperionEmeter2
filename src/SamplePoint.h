#pragma once

#include <stdint.h>
#include <span.h>        // gsl::span
#include "HypCommands.h" // RECORD_LENGTH

class SamplePoint {
    friend std::ostream& operator <<(std::ostream& os, const SamplePoint& obj);
public:
    SamplePoint(const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& data);
private:
    double seconds;
    double volts;
    double amps;
    double mAhOut;
    double mAhIn;
    double RPM;
    double altitude;
    double temp1;
    double temp2;
    double temp3;
    double throttle;
    double tempAmb;

};

std::ostream& operator <<(std::ostream& os, const SamplePoint& obj);
