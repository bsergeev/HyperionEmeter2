#include "Recording.h"
#include "SamplePoint.h"

Recording::Recording(std::vector<SamplePoint>&& points)
    : m_points(points)
{
}

// return whether anything was changed
bool Recording::MassageData()
{
    using vi = SamplePoint::ValueIndex;

    const size_t N_points = m_points.size();
    if (N_points == 0) {
        return false; // nothing to do
    }

    bool changed = false;

    // First, check if the sample rate is less than a second, thus several
    // equal 'seconds' in a row should be fixed.
    size_t max_same_seconds_inarow = 1;
    {
        size_t same_seconds_inarow = 1;
        double prev_seconds = -1.0; // not set
        for (const SamplePoint& pt : m_points) {
            const double& seconds = pt[vi::seconds];
            if (prev_seconds >= 0.0) {
                if (prev_seconds == seconds) {
                    ++same_seconds_inarow;
                    if (max_same_seconds_inarow < same_seconds_inarow) {
                        max_same_seconds_inarow = same_seconds_inarow;
                    }
                } else {
                    prev_seconds = seconds;
                    same_seconds_inarow = 1;
                }
            } else { // it's 1st pass
                prev_seconds = seconds;
            }
        }
    }
    if (max_same_seconds_inarow > 1) 
    {
        // Occasionally, very short sequences don't have a full sub-sequence
        if (max_same_seconds_inarow == 3 && N_points < 7) {
            max_same_seconds_inarow = 4;
        }
        assert(max_same_seconds_inarow == 2 || max_same_seconds_inarow == 4);
        const double ds = 1.0 / max_same_seconds_inarow;

        double prev_seconds = -1.0; // not set
        size_t prev_sec_idx = 0;
        for (size_t i = 0; i < N_points; ++i) {
            const SamplePoint& pt = m_points[i];
            const double& seconds = pt[vi::seconds];
            if (prev_seconds >= 0.0) {
                if (prev_seconds != seconds) { // value just changed
                    if (prev_sec_idx + 1 < i) {
                        changed = true;
                        const double s0 = seconds - (i - prev_sec_idx)*ds;
                        for (size_t j = prev_sec_idx; j < i; ++j) {
                            m_points[j][vi::seconds] =
                                s0 + ds*(j - prev_sec_idx);
                        }
                    }
                    prev_seconds = seconds;
                    prev_sec_idx = i;
                }
            } else { // it's 1st pass
                prev_seconds = seconds;
            }
        }
        if (prev_sec_idx + 1 < N_points) {
            changed = true;
            const double s0 = m_points[prev_sec_idx][vi::seconds];
            for (size_t j = prev_sec_idx+1; j < N_points; ++j) {
                m_points[j][vi::seconds] =
                    s0 + ds*(j - prev_sec_idx);
            }
        }
    }

    // Next, record more accurate discharge values
    const SamplePoint& pt0 = m_points.front();
    double discharged = pt0[vi::mAh_Out];
    double prev_amps  = pt0[vi::amps];
    const double dt = (N_points > 1)? m_points[1][vi::seconds] - pt0[vi::seconds]
                                    : 0.0;
    for (size_t i = 1; i < N_points; ++i) {
        SamplePoint& pt = m_points[i];
        const double orig_discharge = pt[vi::mAh_Out];
        const double& amps = pt[vi::amps];
        pt[vi::mAh_Out] = discharged += 0.5*(prev_amps + amps)*dt;
        changed |= (orig_discharge != pt[vi::mAh_Out]);
        prev_amps = amps;
    }
    return changed;
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
