#ifndef ATXREADER_H
#define ATXREADER_H

#ifdef _WIN32
#include "WinTypes.h"
#endif

#include <memory>
#include <sstream>
#include <vector>

#include <span.h>      // gsl::span

#include <QtWidgets/QDialog>

class DeviceLink;
class Recording;

class QTextEdit;
class QPushButton;

class HypReader : public QDialog
{
    Q_OBJECT
public:
    explicit HypReader(QWidget* parent = nullptr);
   ~HypReader();

   static const size_t RECORD_LENGTH = 26;

signals:
    void MessageToDisplay(const QString& msg);
    void SeriesEnded();
    void DownloadFinish(bool success);

private:
    void DownloadFromDevice();
    void EraseDevice();
    void DisplayMessageFromReader(const QString& msg);
    void EndSeries();
    void FinishDownload(bool success);

    void LoadFromFile();
    void SaveToFile();

    // These get called from DeviceLink worker thread.
    // They emit the data to be processed on UI thread.
    void DisplayMessage  (const QString& msg);
    void MarkSeriesEnd();
    void DownloadFinished(bool success);
    void ReceiveDataChunk(const gsl::span<uint8_t, RECORD_LENGTH>& data);

//data:
    std::unique_ptr<DeviceLink>       m_deviceReader;
    std::vector<std::vector<uint8_t>> m_downloadedRawData;

    std::vector<Recording> m_recordings;

    std::stringstream m_coutStream;
    std::streambuf*   m_cout        = nullptr;
    QTextEdit*        m_textEdit    = nullptr;
    QPushButton*      m_DownloadBtn = nullptr;
    QPushButton*   m_ClearMemoryBtn = nullptr;

//static:
    static void ReadSettings();
    static void WriteSettings();

//static data:
    static const QString BS;
    static bool  kAskForConfgirmation;
};

#endif // ATXREADER_H
