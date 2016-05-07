#include "HypReader.h"
#include "CustomWidgets/YesNoDlg.h"
#include "ComUtils.h"

#include <QtGui>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>

#include <cassert>
#include <fstream>
#include <iomanip> // std::setprecision
#include <iostream>

//static:
bool  HypReader::kAskForConfgirmation = true;
const QString HypReader::BS = QChar(0x10);

//------------------------------------------------------------------------------
HypReader::HypReader(QWidget *parent)
    : QDialog(parent)
{
    ReadSettings();

    setWindowTitle( "Hyperion RDU/MDU/LDU" );

    QVBoxLayout* layout = new QVBoxLayout(this);

    m_textEdit = new QTextEdit(this);
    layout->addWidget( m_textEdit );

    QHBoxLayout* lay1 = new QHBoxLayout;
    lay1->addStretch( 2 );

    m_DownloadBtn = new QPushButton(tr(" Download data "));
    connect(m_DownloadBtn, &QPushButton::clicked, [this]{ DownloadFromDevice(); } );
    lay1->addWidget(m_DownloadBtn);
    lay1->addStretch( 1 );

    m_ClearMemoryBtn = new QPushButton(tr(" Clear RDU/MDU "));
    connect(m_ClearMemoryBtn, &QPushButton::clicked, [this]{ EraseDevice(); });
    lay1->addWidget(m_ClearMemoryBtn);
    lay1->addStretch( 1 );

    QPushButton* pb = new QPushButton(tr("Close"));
    connect(pb, SIGNAL(clicked()), this, SLOT(reject()));
    lay1->addWidget( pb );

    lay1->addStretch( 2 );
    layout->addLayout(lay1);

    resize(QSize(1000, 750));

    connect(this, &HypReader::MessageToDisplay, this, &HypReader::DisplayMessageFromReader);
    connect(this, &HypReader::SeriesEnded,      this, &HypReader::EndSeries);
    connect(this, &HypReader::DownloadFinish,   this, &HypReader::FinishDownload);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HypReader::~HypReader()
{
    WriteSettings();
}
//------------------------------------------------------------------------------
void HypReader::DisplayMessageFromReader(const QString& msg)
{
    if (!msg.isEmpty()) {
        QString message = msg;
        if (msg[0] == BS) {
            m_textEdit->moveCursor(QTextCursor::End,         QTextCursor::MoveAnchor);
            m_textEdit->moveCursor(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            m_textEdit->moveCursor(QTextCursor::Up,          QTextCursor::KeepAnchor);
            m_textEdit->textCursor().removeSelectedText();

            message.remove(0, 1);
        }
        m_textEdit->append(message);
    }
}
//------------------------------------------------------------------------------
void HypReader::EndSeries()
{
    m_DownloadedData.push_back(std::vector<uint8_t>());
}
//------------------------------------------------------------------------------
void HypReader::FinishDownload(bool success)
{
    auto twoUChars2Int = [](uint8_t hi, uint8_t lo) {
        return static_cast<int16_t>((hi << 8) | lo);
    };

    if (success)
    {
        m_cout = std::cout.rdbuf(m_coutStream.rdbuf());
        m_coutStream.clear(); m_coutStream.str("");

        std::cout << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
        std::cout << "Seconds\tVoltage, V\tCurrent, A\tDisch, mAh\tCharge, mAh\tRPM\tAltitude, m\tTemp1, C\tTemp2, C\tTemp3, C\tThrottle, uS\tAmb.Temp, C\n";
        for (const std::vector<uint8_t>& sessionData : m_DownloadedData)
        {
            assert(sessionData.size() % RECORD_LENGTH == 0);
            std::cout << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
            const size_t DATA_SIZE = sessionData.size();
            for (size_t recordAddr = 0; recordAddr < DATA_SIZE; recordAddr += RECORD_LENGTH)
            {
                const uint8_t* record = &sessionData[recordAddr];
                const double volts   = twoUChars2Int(record[ 0], record[ 1]) / 100.0;
                const double amps    = twoUChars2Int(record[ 2], record[ 3]) / 100.0;
                const int    mAhIn   = twoUChars2Int(record[ 4], record[ 5]);
                const int    mAhOut  = twoUChars2Int(record[ 6], record[ 7]);
                const int    RPM     = twoUChars2Int(record[ 8], record[ 9]);
                const int    seconds = twoUChars2Int(record[10], record[11]);
                const int    altitude= twoUChars2Int(record[12], record[13]);        // <<< TBD: verify negative?
                const double temp1   = twoUChars2Int(record[14], record[15]) / 10.0;
                const double temp2   = twoUChars2Int(record[16], record[17]) / 10.0;
                const double temp3   = twoUChars2Int(record[18], record[19]) / 10.0;
                const int    throttle= twoUChars2Int(record[20], record[21]);
                const double tempAmb = twoUChars2Int(record[22], record[23]) / 10.0; // <<< TBD: verify negative
                static const char* s = "\t"; // separator
                std::cout << seconds << s 
                          << std::fixed << std::setprecision(2) << volts << s
                          << std::fixed << std::setprecision(2) << amps  << s 
                          << mAhIn << s << mAhOut << s << RPM << s << altitude << s
                          << std::fixed << std::setprecision(1) << temp1 << s 
                          << std::fixed << std::setprecision(1) << temp2 << s 
                          << std::fixed << std::setprecision(1) << temp3 << s 
                          << throttle << s 
                          << std::fixed << std::setprecision(1) << tempAmb << "\n";
            }
        }
        m_textEdit->clear();
        m_textEdit->setText(QString::fromStdString(m_coutStream.str()));
        std::cout.rdbuf(m_cout);
    //} else { // failure
    //    QMessageBox::critical(this, tr("Download Error"), tr("Data download failed!"));
    }
    m_DownloadBtn->setEnabled(true);
}
//------------------------------------------------------------------------------
// These gets called from DeviceLink worker thread:
void HypReader::DisplayMessage(const QString& m) { emit MessageToDisplay(m); }
void HypReader::MarkSeriesEnd()                  { emit SeriesEnded(); }
void HypReader::DownloadFinished(bool success)   { emit DownloadFinish(success); }

void HypReader::ReceiveDataChunk(const DeviceLink::packet_t& data)
{
    // Add this chunk to the main data vector
    std::vector<uint8_t>& sessionData = m_DownloadedData.back();
    const bool firstPoint = sessionData.empty();
    std::copy(data.begin(), data.end(), std::back_inserter(sessionData));

    emit MessageToDisplay(((firstPoint)? QString("") : BS) 
                         + tr("Received point %1").arg(sessionData.size()/26));
}
//------------------------------------------------------------------------------
void HypReader::DownloadFromDevice()
{
    bool download = true;
    //if (kAskForConfgirmation)
    //{
    //    YesNoDlg dlg(tr("Data Download")
    //                , tr("Blah, blah, blah")
    //                , tr("Don't show this again")
    //                , true  // show Don't ask
    //                , QMessageBox::Ok | QMessageBox::Cancel
    //                , this );
    //    download = (dlg.exec() == QDialog::Accepted);
    //    kAskForConfgirmation = !dlg.IsDontAskAgainSelected();
    //}
    if (download)
    {
        m_DownloadBtn->setEnabled(false);

        m_DownloadedData.clear();
        m_DownloadedData.push_back(std::vector<uint8_t>());

        m_deviceReader.reset(new DeviceLink([this](const QString&  msg)           { DisplayMessage(msg);  },
                                            [this](const DeviceLink::packet_t& p) { ReceiveDataChunk(p);  },
                                            [this]()                              { MarkSeriesEnd();      },
                                            [this](bool ok)                       { DownloadFinished(ok); }));
        m_deviceReader->DownloadRecorded(); // start downloading (non-blocking call)

        m_textEdit->clear(); // prepare for process messages
    }
}
//------------------------------------------------------------------------------
void HypReader::EraseDevice()
{
    bool doErase = true;
    if (kAskForConfgirmation) {
        YesNoDlg dlg( tr("Erase")
                    , tr("Do you really want to erase all the data?")
                    , tr("Don't show this again")
                    , true  // show Don't ask
                    , QMessageBox::Yes | QMessageBox::No
                    , this );
        doErase = (dlg.exec() == QDialog::Accepted);
        kAskForConfgirmation = !dlg.IsDontAskAgainSelected();
    }
    if (doErase)
    {
        m_DownloadBtn->setEnabled(false);

        m_DownloadedData.clear();
        m_DownloadedData.push_back(std::vector<uint8_t>());

        m_deviceReader.reset(new DeviceLink([this](const QString&  msg)         { DisplayMessage(msg);  },
                                            [this](const DeviceLink::packet_t&) {                       },
                                            [this]()                            { MarkSeriesEnd();      },
                                            [this](bool ok)                     { DownloadFinished(ok); }));
        m_deviceReader->ClearRecordings(); // start erasing (non-blocking call)

        m_textEdit->clear(); // prepare for process messages
    }
}
//------------------------------------------------------------------------------
void HypReader::LoadFromFile()
{
    // TBD
}
//------------------------------------------------------------------------------
void HypReader::SaveToFile()
{
    // TBD
}
//------------------------------------------------------------------------------
// Two methods for reading and saving static variables and current window size/position
void HypReader::ReadSettings()
{
    static bool alreadyLoaded = false;
    if (!alreadyLoaded)
    {
        QSettings settings;
        kAskForConfgirmation = settings.value("ShowInstructions", kAskForConfgirmation).toBool();
        alreadyLoaded = true;
    }
}

void HypReader::WriteSettings()
{
    QSettings settings;

    settings.setValue("ShowInstructions", kAskForConfgirmation);
}
//------------------------------------------------------------------------------