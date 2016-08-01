#include "MainWnd.h"

#include "LogWindow.h"
#include "Recording.h"
#include "RecordingDataModel.h"
#include "RecordingDisplayWnd.h"
#include "CustomWidgets/YesNoDlg.h"

#include <QtWidgets>

// static
bool   MainWnd::kAskForConfgirmation = true;

MainWnd::MainWnd(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_reader(std::make_unique<HypReader>([this](const QString& s){ DisplayMessage(s); },
                                           [this](size_t numRsDled){ DownloadFinished(numRsDled); },
                                           this))
    , m_mdiArea(new QMdiArea)
{
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy  (Qt::ScrollBarAsNeeded);
    setCentralWidget(m_mdiArea);
    connect(m_mdiArea, &QMdiArea::subWindowActivated, this, &MainWnd::updateMenus);



    m_fileMenu = menuBar()->addMenu(tr("&File"));
    if (m_actOpen = new (std::nothrow) QAction(tr("Open"), this)) {
        m_actOpen->setToolTip(tr("Open recording from disk"));
        m_actOpen->setShortcut(QKeySequence::Open);
        m_actOpen->setIcon(QIcon(":/images/toolbars/button_open.png"));
        connect(m_actOpen, &QAction::triggered, this, &MainWnd::LoadFromFile);
        m_fileMenu->addAction(m_actOpen);
    }
    if (m_actSave = new (std::nothrow) QAction(tr("Save"), this)) {
        m_actSave->setToolTip(tr("Save recording to disk"));
        m_actSave->setShortcut(QKeySequence::Save);
        m_actSave->setIcon(QIcon(":/images/toolbars/button_save.png"));
        connect(m_actSave, &QAction::triggered, this, &MainWnd::SaveToFile);
        m_fileMenu->addAction(m_actSave);
    }
    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    if (QAction* exitAct = m_fileMenu->addAction(exitIcon, tr("E&xit"), qApp, &QApplication::closeAllWindows)) {
        exitAct->setShortcuts(QKeySequence::Quit);
        exitAct->setStatusTip(tr("Exit the application"));
        m_fileMenu->addSeparator();
        m_fileMenu->addAction(exitAct);
    }

    m_deviceMenu = menuBar()->addMenu(tr("&Device"));
    if (m_actDownload = new (std::nothrow) QAction(tr("Download"), this)) {
        m_actDownload->setToolTip(tr("Download recordings from RDU/MDU"));
        m_actDownload->setIcon(QIcon(":/images/toolbars/button_download.png"));
        connect(m_actDownload, &QAction::triggered, this, &MainWnd::DownloadFromDevice);
        m_deviceMenu->addAction(m_actDownload);
    }
    if (m_actErase = new (std::nothrow) QAction(tr("Erase"), this)) {
        m_actErase->setToolTip(tr("Erase RDU/MDU"));
        m_actErase->setIcon(QIcon(":/images/toolbars/button_erase.png"));
        connect(m_actErase, &QAction::triggered, this, &MainWnd::EraseDevice);
        m_deviceMenu->addAction(m_actErase);
    }

    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    connect(m_windowMenu, &QMenu::aboutToShow, this, &MainWnd::updateMenus);

    m_toolBar = addToolBar("Operations");
    m_toolBar->setAttribute(Qt::WA_AcceptTouchEvents);
    m_toolBar->setMovable(false);
    m_toolBar->addAction(m_actDownload);
    m_toolBar->addAction(m_actErase);
    m_toolBar->addAction(m_actOpen);
    m_toolBar->addAction(m_actSave);

    if (QAction* exitAct = new (std::nothrow) QAction(tr("E&xit"), this)) {
        exitAct->setShortcuts(QKeySequence::Quit);
        exitAct->setStatusTip(tr("Exit the application"));
        connect(exitAct, &QAction::triggered, this, &QWidget::close);
    }


    if (m_logWindow = new (std::nothrow) LogWindow(this)) {
        m_mdiArea->addSubWindow(m_logWindow);
    }

    updateMenus();
    readSettings();

    connect(this, &MainWnd::MessageToDisplay, this, &MainWnd::DisplayMessageFromReader);
    connect(this, &MainWnd::SignalDLFinished, this, &MainWnd::FinishDownload);
}

MainWnd::~MainWnd() {
    writeSettings();
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
    QString title  = tr("Save recordings to a file");
    QString filePath = QFileDialog::getSaveFileName(this, title, ".", filter);
    if (!filePath.isEmpty()) // empty means 'Cancel' was hit
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
void MainWnd::DisplayMessage  (const QString& msg)  { emit MessageToDisplay(msg); }
void MainWnd::DownloadFinished(size_t NRecsDwnlded) { emit SignalDLFinished(NRecsDwnlded); }

// Receives the message on the UI thread
void MainWnd::DisplayMessageFromReader(const QString& msg)
{
    if (!msg.isEmpty()) {
        m_logWindow->AddLine(msg);
    }
}

// Instantiate result windows on the UI thread
void MainWnd::FinishDownload(size_t N_recsDLed)
{
    if (N_recsDLed == 0) {
        m_logWindow->AddLine(tr("Failed to download any recordings"));
    } else {
        m_logWindow->AddLine(tr("Downloaded %1 recording%2").arg(N_recsDLed)
                                                            .arg((N_recsDLed > 1)? "s":""));
        for (size_t i = 0; i < N_recsDLed; ++i) {
            if (m_reader->GetRecording(i).size() > 0) 
            {
                auto dataModel = std::make_shared<RecordingDataModel>(m_reader->GetRecording(i), this);
                if (auto w = new RecordingDisplayWnd(dataModel, this)) {
                    w->setWindowTitle(QObject::tr("Recording %1").arg(i + 1));
                    w->setWindowIcon(QIcon(":/images/face-smile.png"));

                    m_mdiArea->addSubWindow(w);
                    if (N_recsDLed == 1) {
                        w->showMaximized();
                    } else {
                        w->show();
                    }
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void MainWnd::updateMenus()
{
    m_windowMenu->clear();

    if (QMdiSubWindow* activeSubWindow = m_mdiArea->activeSubWindow()) {
        if (auto rdw = dynamic_cast<RecordingDisplayWnd*>(activeSubWindow->widget())) {
            m_windowMenu->addAction(rdw->GetGraphOptionsAction());
        }
    }
}
//------------------------------------------------------------------------------
// Two methods for reading and saving static variables and current window size/position
void MainWnd::readSettings()
{
    static bool alreadyLoaded = false;
    if (!alreadyLoaded)
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

        const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
        if (geometry.isEmpty()) {
            const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
            resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
            move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
        } else {
            restoreGeometry(geometry);
        }

        kAskForConfgirmation = settings.value("ShowInstructions", kAskForConfgirmation).toBool();
        alreadyLoaded = true;
    }
}

void MainWnd::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("ShowInstructions", kAskForConfgirmation);
}
