#include "MainWnd.h"

#include "LogWindow.h"
#include "CustomWidgets/YesNoDlg.h"

#include <QtWidgets>

// static
bool MainWnd::kAskForConfgirmation = true;

MainWnd::MainWnd(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_reader(std::make_unique<HypReader>([this](const QString& s){ DisplayMessage(s); }, this))
    , m_mdiArea(new QMdiArea)
{
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy  (Qt::ScrollBarAsNeeded);
    setCentralWidget(m_mdiArea);
    connect(m_mdiArea, &QMdiArea::subWindowActivated, this, &MainWnd::updateMenus);


    if (m_actDownload = new (std::nothrow) QAction(tr("Download"), this)) {
        m_actDownload->setToolTip(tr("Download recordings from RDU/MDU"));
        m_actDownload->setIcon(QIcon(":/images/toolbars/button_download.png"));
        connect(m_actDownload, &QAction::triggered, this, &MainWnd::DownloadFromDevice);
    }

    if (m_actErase = new (std::nothrow) QAction(tr("Erase"), this)) {
        m_actErase->setToolTip(tr("Erase RDU/MDU"));
        m_actErase->setIcon(QIcon(":/images/toolbars/button_erase.png"));
        connect(m_actErase, &QAction::triggered, this, &MainWnd::EraseDevice);
    }

    if (m_actOpen = new (std::nothrow) QAction(tr("Open"), this)) {
        m_actOpen->setToolTip(tr("Open recording from disk"));
        m_actOpen->setShortcut(tr("Ctrl+O"));
        m_actOpen->setIcon(QIcon(":/images/toolbars/button_open.png"));
        connect(m_actOpen, &QAction::triggered, this, &MainWnd::LoadFromFile);
    }

    if (m_actSave = new (std::nothrow) QAction(tr("Save"), this)) {
        m_actSave->setToolTip(tr("Save recording to disk"));
        m_actSave->setShortcut(tr("Ctrl+S"));
        m_actSave->setIcon(QIcon(":/images/toolbars/button_save.png"));
        connect(m_actSave, &QAction::triggered, this, &MainWnd::SaveToFile);
    }

    m_toolBar = addToolBar("Operations");
    m_toolBar->setAttribute(Qt::WA_AcceptTouchEvents);
    m_toolBar->setMovable(false);
    m_toolBar->addAction(m_actDownload);
    m_toolBar->addAction(m_actErase);
    m_toolBar->addAction(m_actOpen);
    m_toolBar->addAction(m_actSave);

    if (m_logWindow = new (std::nothrow) LogWindow(this)) {
        m_mdiArea->addSubWindow(m_logWindow);
    }

    updateMenus();

    connect(this, &MainWnd::MessageToDisplay, this, &MainWnd::DisplayMessageFromReader);
}
MainWnd::~MainWnd() {
    WriteSettings();
}
//------------------------------------------------------------------------------
void MainWnd::DownloadFromDevice()
{
    //m_DownloadBtn->setEnabled(false);
    m_reader->DownloadFromDevice();
}

void MainWnd::EraseDevice()
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
        //m_DownloadBtn->setEnabled(false);
        m_reader->EraseDevice();
    }
}

void MainWnd::LoadFromFile()
{
    // TBD
}

void MainWnd::SaveToFile()
{
    QString filter = tr("Tab Separated Values (*.tsv)");
    QString title = tr("Save recordings to a file");
    QString filePath = QFileDialog::getSaveFileName(this, title, ".", filter);
    if (!filePath.isEmpty()) // empty name means 'Cancel' was hit
    {
        const bool ok = m_reader->SaveToFile(filePath);
        if (ok) {
            m_logWindow->AddLine(tr("Saved recording(s) to \"%1\"").arg(filePath));
        } else {
            m_logWindow->AddLine(tr("Failed to save recording(s) to \"%1\"").arg(filePath));
            QMessageBox::critical(this, "Save Error", "Saving the recording failed!");
        }
    }
}
//------------------------------------------------------------------------------
// Called from a worker thread
void MainWnd::DisplayMessage(const QString& m) { emit MessageToDisplay(m); }

// Receives the message on the UI thread
void MainWnd::DisplayMessageFromReader(const QString& msg)
{
    if (!msg.isEmpty()) {
        m_logWindow->AddLine(msg);
    }
}
//------------------------------------------------------------------------------
void MainWnd::updateMenus()
{
}
//------------------------------------------------------------------------------
// Two methods for reading and saving static variables and current window size/position
void MainWnd::ReadSettings()
{
    static bool alreadyLoaded = false;
    if (!alreadyLoaded)
    {
        QSettings settings;
        kAskForConfgirmation = settings.value("ShowInstructions", kAskForConfgirmation).toBool();
        alreadyLoaded = true;
    }
}

void MainWnd::WriteSettings()
{
    QSettings settings;

    settings.setValue("ShowInstructions", kAskForConfgirmation);
}
