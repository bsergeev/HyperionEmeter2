#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <span.h>       // gsl::span

#include <QObject>

#include "HypCommands.h" // Hyperion::RECORD_LENGTH

class DeviceLink;
class Recording;

class HypReader : public QObject
{
    Q_OBJECT
public:
    explicit HypReader(std::function<void(const QString&)> msgCBck,
                       QObject* parent = nullptr);

    void DownloadFromDevice();
    void EraseDevice();

    bool LoadFromFile(const QString& filePath);
    bool SaveToFile  (const QString& filePath);

signals:
    void SeriesEnded();
    void DownloadFinish(bool success);

private:
    void EndSeries();
    void FinishDownload(bool success);

    // These get called from DeviceLink worker thread.
    // They emit the data to be processed on UI thread.
    void MarkSeriesEnd();
    void DownloadFinished(bool success);
    void ReceiveDataChunk(const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& data);

//data:
    std::function<void(const QString&)> m_msgCBck;
    std::unique_ptr<DeviceLink>         m_deviceReader;

    std::vector<std::vector<uint8_t>> m_downloadedRawData;
    std::vector<Recording>            m_recordings;

public:
    static const QString BS;
};
