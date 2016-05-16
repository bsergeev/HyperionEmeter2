#include "HypReader.h"
#include "Recording.h"
#include "SamplePoint.h"
#include "ComUtils.h"

#include <span.h>  // span

#include <cassert>
#include <fstream>
#include <iostream>

//static:
const QString HypReader::BS = QChar(0x10);

//------------------------------------------------------------------------------
HypReader::HypReader(std::function<void(const QString&)> msgCBk, QObject* parent)
    : QObject(parent)
    , m_msgCBck(msgCBk)
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
            
        //Recording::PrintHeader(std::cout);
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

            m_recordings.emplace_back(Recording{ std::move(points) });
            m_recordings.back().MassageData();

            //std::cout << m_recordings.back();
        }
    } else { // failure
        if (m_msgCBck) {
            m_msgCBck(tr("Download failed!"));
        }
    }
//    m_DownloadBtn->setEnabled(true);
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
                               [this]()        {},
                               [this](bool ok) {}));
    m_deviceReader->ClearRecordings(); // start erasing (non-blocking call)
}
//------------------------------------------------------------------------------
bool HypReader::LoadFromFile(const QString& filePath)
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
                recording.PrintHeader(file);
                recording.PrintData  (file);
                file << std::endl;
            }
            ok = !file.fail();
        }
    }
    return ok;
}
//------------------------------------------------------------------------------