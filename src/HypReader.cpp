#include "HypReader.h"
#include "Recording.h"
#include "SamplePoint.h"
#include "ComUtils.h"

#include <span.h>  // span

#include <cassert>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

//static:
const QString HypReader::BS = QChar(0x10);

//------------------------------------------------------------------------------
HypReader::HypReader(std::function<void(const QString&)> msgCBk, 
                     std::function<void(size_t)> finishedCBck,
                     QObject* parent)
    : QObject(parent)
    , m_msgCBck(msgCBk)
    , m_finishedCBck(finishedCBck)
{
    connect(this, &HypReader::SeriesEnded,    this, &HypReader::EndSeries);
    connect(this, &HypReader::DownloadFinish, this, &HypReader::FinishDownload);
}
//------------------------------------------------------------------------------
void HypReader::EndSeries()
{
    m_downloadedRawData.push_back(std::vector<uint8_t>());
}
//------------------------------------------------------------------------------
void HypReader::FinishDownload(bool success)
{
    if (success)
    {
        m_recordings.clear();

        for (auto& sessionData : m_downloadedRawData)
        {
            const size_t DATA_SIZE = sessionData.size();
            assert(DATA_SIZE % Hyperion::RECORD_LENGTH == 0);

            std::vector<SamplePoint> points;
            points.reserve(DATA_SIZE / Hyperion::RECORD_LENGTH);

            //Recording::PrintDivider(std::cout);
            for (size_t recordAddr=0; recordAddr+Hyperion::RECORD_LENGTH-1 < DATA_SIZE; recordAddr += Hyperion::RECORD_LENGTH) {
                points.emplace_back(gsl::span<uint8_t, Hyperion::RECORD_LENGTH>
                                    { &sessionData[recordAddr], static_cast<int64_t>(Hyperion::RECORD_LENGTH) });
            }

            // Compose recording title
            // Get download time
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            std::ostringstream timeSS;
            timeSS << std::put_time(&tm, " on %A %F at %X");
            
            const int fwVersion = GetFirmwareVersionX100();
            const int majV = fwVersion / 100;
            const int minV = fwVersion % 100;
            const std::string title = std::string("Recording ") + std::to_string(m_recordings.size()+1)
                + " downloaded from " + Hyperion::DEVICE_NAME[GetDeviceType()]
                + " v"+ std::to_string(majV) +"."+ ((minV < 10)? "0":"") + std::to_string(minV)
                + timeSS.str();
            m_recordings.emplace_back(title, std::move(points));
            m_recordings.back().MassageData();
        }
        if (!m_recordings.empty() && m_recordings.back().size() == 0) {
            m_recordings.pop_back();
        }
        // Notify the caller that m_recordings.size() recordings have been downloaded
        m_finishedCBck(m_recordings.size());
    } 
    else { // failure
        if (m_msgCBck) {
            m_msgCBck(tr("Download failed!"));
        }
    }
}
//------------------------------------------------------------------------------
// These gets called from DeviceLink worker thread:
void HypReader::MarkSeriesEnd()           { emit SeriesEnded();      }
void HypReader::DownloadFinished(bool ok) { emit DownloadFinish(ok); }

void HypReader::ReceiveDataChunk(const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& data)
{
    // Add this chunk to the main data vector
    std::vector<uint8_t>& sessionData = m_downloadedRawData.back();
    const bool firstPoint = sessionData.empty();
    std::copy(data.begin(), data.end(), std::back_inserter(sessionData));

    if (m_msgCBck) {
        m_msgCBck(((firstPoint)? QString("") : BS) 
                  + tr("Received point %1").arg(sessionData.size()/Hyperion::RECORD_LENGTH));
    }
}
//------------------------------------------------------------------------------
void HypReader::DownloadFromDevice()
{
    // Don't clear m_recordings until download finishes
    m_downloadedRawData.clear();
    m_downloadedRawData.push_back(std::vector<uint8_t>());

    m_deviceReader.reset(new DeviceLink(m_msgCBck,
                            [this](const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& p) { ReceiveDataChunk(p); },
                            [this]()        { MarkSeriesEnd();      },
                            [this](bool ok) { DownloadFinished(ok); }));
    m_deviceReader->DownloadRecorded(); // start downloading (non-blocking call)
}
//------------------------------------------------------------------------------
void HypReader::EraseDevice()
{
    m_downloadedRawData.clear();
    m_downloadedRawData.push_back(std::vector<uint8_t>());

    m_deviceReader.reset(new DeviceLink(m_msgCBck,
                               [this](const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>&) {},
                               [this]()     {},
                               [this](bool) {}));
    m_deviceReader->ClearRecordings(); // start erasing (non-blocking call)
}
//------------------------------------------------------------------------------
bool HypReader::LoadFromFile(const QString&)// filePath)
{
    // TBD
    return false;
}
//------------------------------------------------------------------------------
bool HypReader::SaveToFile(const QString& filePath)
{
    bool ok = false;
    if (!filePath.isEmpty()) {
        std::ofstream file(filePath.toStdString(), std::ios_base::out);
        if ((ok = (file.good() && file.is_open())) == true) {
            for (auto& recording : m_recordings) {
                file << recording.GetTitle() << std::endl;
                file << recording.GetSubtitle() << std::endl;
                recording.PrintColumnNames(file);
                recording.PrintData       (file);
                file << std::endl;
            }
            ok = !file.fail();
        }
    }
    return ok;
}
//------------------------------------------------------------------------------
size_t HypReader::GetNumRecordings() const 
{ 
    return m_recordings.size(); 
}

const Recording& HypReader::GetRecording(size_t idx) const
{
    if (idx < m_recordings.size()) {
        return m_recordings.at(idx);
    } else {
        assert(!"Invalid recording index");
        static const Recording error{ "ERROR", std::vector<SamplePoint>() };
        return error;
    }
}
//------------------------------------------------------------------------------
Hyperion::DeviceType HypReader::GetDeviceType() const
{ 
    if (DeviceLink* const device = m_deviceReader.get()) {
        return device->GetDeviceType();
    }
    assert(!"DeviceLink not instantiated");
    return Hyperion::DeviceType::UNKNOWN_DEVICE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int HypReader::GetFirmwareVersionX100() const 
{ 
    if (DeviceLink* const device = m_deviceReader.get()) {
        return device->GetFirmwareVersionX100();
    }
    assert(!"DeviceLink not instantiated");
    return 0; // unknown 
}

//------------------------------------------------------------------------------