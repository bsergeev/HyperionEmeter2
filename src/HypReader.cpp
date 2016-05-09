#include "HypReader.h"
#include "Recording.h"
#include "SamplePoint.h"
#include "CustomWidgets/YesNoDlg.h"
#include "ComUtils.h"

#include <QtGui>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>

#include <span.h>  // span

#include <cassert>
#include <fstream>
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

    QPushButton* pb = new QPushButton(tr(" Save recordings to file "));
    connect(pb, &QPushButton::clicked, [this] { SaveToFile(); });
    lay1->addWidget(pb);
    lay1->addStretch(1);

    pb = new QPushButton(tr("Close"));
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
    m_downloadedRawData.push_back(std::vector<uint8_t>());
}
//------------------------------------------------------------------------------
void HypReader::FinishDownload(bool success)
{
    if (success)
    {
        m_recordings.clear();
            
        m_cout = std::cout.rdbuf(m_coutStream.rdbuf());
        m_coutStream.clear(); m_coutStream.str("");

        Recording::PrintHeader(std::cout);
        for (auto& sessionData : m_downloadedRawData)
        {
            const size_t DATA_SIZE = sessionData.size();
            assert(DATA_SIZE % Hyperion::RECORD_LENGTH == 0);

            std::vector<SamplePoint> points;
            points.reserve(DATA_SIZE / Hyperion::RECORD_LENGTH);

            Recording::PrintDivider(std::cout);
            for (size_t recordAddr=0; recordAddr+Hyperion::RECORD_LENGTH-1 < DATA_SIZE; recordAddr += Hyperion::RECORD_LENGTH) {
                points.emplace_back(gsl::span<uint8_t, Hyperion::RECORD_LENGTH>
                                    { &sessionData[recordAddr], static_cast<int64_t>(Hyperion::RECORD_LENGTH) });
            }

            m_recordings.emplace_back(Recording{ std::move(points) });
            m_recordings.back().MassageData();

            std::cout << m_recordings.back();
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
void HypReader::MarkSeriesEnd()                  { emit SeriesEnded();       }
void HypReader::DownloadFinished(bool ok)        { emit DownloadFinish(ok);  }

void HypReader::ReceiveDataChunk(const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& data)
{
    // Add this chunk to the main data vector
    std::vector<uint8_t>& sessionData = m_downloadedRawData.back();
    const bool firstPoint = sessionData.empty();
    std::copy(data.begin(), data.end(), std::back_inserter(sessionData));

    emit MessageToDisplay(((firstPoint)? QString("") : BS) 
                         + tr("Received point %1").arg(sessionData.size()/Hyperion::RECORD_LENGTH));
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
    //                , QMessageBox::Ok | QMessageBox::Cancel
    //                , this );
    //    download = (dlg.exec() == QDialog::Accepted);
    //    kAskForConfgirmation = !dlg.IsDontAskAgainSelected();
    //}
    if (download)
    {
        m_DownloadBtn->setEnabled(false);

        // Don't clear m_recordings until download finishes
        m_downloadedRawData.clear();
        m_downloadedRawData.push_back(std::vector<uint8_t>());

        m_deviceReader.reset(new DeviceLink([this](const QString&  msg){ DisplayMessage(msg);  },
                               [this](const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>& p) { ReceiveDataChunk(p); },
                               [this]()                                { MarkSeriesEnd();      },
                               [this](bool ok)                         { DownloadFinished(ok); }));
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
                    , QMessageBox::Yes | QMessageBox::No
                    , this );
        doErase = (dlg.exec() == QDialog::Accepted);
        kAskForConfgirmation = !dlg.IsDontAskAgainSelected();
    }
    if (doErase)
    {
        m_DownloadBtn->setEnabled(false);

        m_downloadedRawData.clear();
        m_downloadedRawData.push_back(std::vector<uint8_t>());

        m_deviceReader.reset(new DeviceLink([this](const QString&  msg)         { DisplayMessage(msg);  },
                                            [this](const gsl::span<uint8_t, Hyperion::RECORD_LENGTH>&) {          },
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
    QString filter = tr("Tab Separated Values (*.tsv)");
    QString title = tr("Save recordings to a file");
    QString fileName = QFileDialog::getSaveFileName(this, title, ".", filter);
    if (!fileName.isEmpty()) // empty name means 'Cancel' was hit
    {
        std::ofstream file(fileName.toStdString(), std::ios_base::out);
        bool ok = (file.good() && file.is_open());
        if (ok) {
            for (auto& recording : m_recordings) {
                file << recording << "\n";
            }
            ok = !file.fail();
        }
        if (!ok) {
            QMessageBox::critical(this, "Save Error", "Saving the recording failed!");
        }
    }
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