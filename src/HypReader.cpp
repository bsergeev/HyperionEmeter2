#include <cassert>
#include <fstream>
#include <iomanip> // std::setprecision
#include <iostream>

#include <QtGui>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>

#include "HypReader.h"
#include "CustomWidgets/YesNoDlg.h"
#include "ComUtils.h"

//static:
bool  AtxReader::kShowDownloadInstructions = true;
const QString AtxReader::BS = QChar(0x10);

int16_t twoUChars2Int(uint8_t hi, uint8_t lo) {
    const int16_t r = static_cast<int16_t>((hi << 8) | lo);
    return r;
}

//------------------------------------------------------------------------------
AtxReader::AtxReader(QWidget *parent)
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

    connect(this, &AtxReader::MessageToDisplay, this, &AtxReader::DisplayMessageFromReader);
    connect(this, &AtxReader::SeriesEnded,      this, &AtxReader::EndSeries);
    connect(this, &AtxReader::DownloadFinish,   this, &AtxReader::FinishDownload);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
AtxReader::~AtxReader()
{
    WriteSettings();
}
//------------------------------------------------------------------------------
void AtxReader::DisplayMessageFromReader(const QString& msg)
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
void AtxReader::EndSeries()
{
    m_DownloadedData.push_back(std::vector<uint8_t>());
}
//------------------------------------------------------------------------------
void AtxReader::FinishDownload(bool success)
{
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
// This gets called from DeviceLink worker thread.
void AtxReader::DisplayMessage(const QString& msg)
{
    emit MessageToDisplay(msg);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// This gets called from DeviceLink worker thread.
void AtxReader::ReceiveDataChunk(const DeviceLink::packet_t& data)
{
    // Add this chunk to the main data vector
    const uint8_t* const d = &data[0];
    std::vector<uint8_t>& sessionData = m_DownloadedData.back();
    const bool firstPoint = sessionData.empty();
    sessionData.insert(sessionData.end(), d, d + data.size());

    QString msg = (firstPoint)? QString("") : BS;
    emit MessageToDisplay(msg + tr("Received point %1").arg(sessionData.size()/26));
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// This gets called from DeviceLink worker thread.
void AtxReader::MarkSeriesEnd()
{
    emit SeriesEnded();
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// This gets called from DeviceLink worker thread.
void AtxReader::DownloadFinished(bool success)
{
    emit DownloadFinish(success);
}
//------------------------------------------------------------------------------
void AtxReader::DownloadFromDevice()
{
    bool download = true;
    if (false) // kShowDownloadInstructions)
    {
        YesNoDlg dlg(tr("Model Download")
                    , tr("To download a model from your transmitter, click Ok,<br>"
                         "then click SYSTEM/18.DATA TRANSFER/(make sure TX -> PC is selected)/<br>"
                         "ENTER/Yes on the TX.")
                    , tr("Don't show this again")
                    , true  // show Don't ask
                    , QMessageBox::Ok | QMessageBox::Cancel
                    , this );
        download = (dlg.exec() == QDialog::Accepted);
        kShowDownloadInstructions = !dlg.IsDontAskAgainSelected();
    }
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
void AtxReader::EraseDevice()
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
//------------------------------------------------------------------------------
void AtxReader::LoadFromFile()
{
    // TBD
}
//------------------------------------------------------------------------------
void AtxReader::SaveToFile()
{
    // TBD
}
//------------------------------------------------------------------------------
// Two methods for reading and saving static variables and current window size/position
void AtxReader::ReadSettings()
{
    static bool alreadyLoaded = false;
    if (!alreadyLoaded)
    {
        QSettings settings;
        kShowDownloadInstructions = settings.value("ShowInstructions", kShowDownloadInstructions).toBool();
        alreadyLoaded = true;
    }
}

void AtxReader::WriteSettings()
{
    QSettings settings;

    settings.setValue("ShowInstructions", kShowDownloadInstructions);
}
//------------------------------------------------------------------------------