#pragma once

#include "HypReader.h"

#include <QMainWindow>
#include <QColor>

#include <memory>

class QWidget;
class QMdiArea;
class QToolBar;
class QAction;
class QString;

class LogWindow;

//------------------------------------------------------------------------------

class MainWnd : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWnd(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~MainWnd();

signals:
    void MessageToDisplay(const QString& msg);
    void SignalDLFinished(size_t numRecsDownloaded);

private:
    void DownloadFromDevice();
    void EraseDevice ();
    void LoadFromFile();
    void SaveToFile  ();
    void updateMenus ();
    void readSettings();
    void writeSettings();

    void DisplayMessageFromReader(const QString& msg);
    void FinishDownload(size_t numRecsDownloaded);

    // These get called from DeviceLink worker thread.
    // They emit the data to be processed on UI thread.
    void DisplayMessage(const QString& msg);
    void DownloadFinished(size_t numRecsDownloaded);

//data:
    std::unique_ptr<HypReader> m_reader;

    QMdiArea* m_mdiArea     = nullptr;
    QMenu*    m_fileMenu    = nullptr;
    QMenu*    m_deviceMenu  = nullptr;
    QMenu*    m_windowMenu  = nullptr;
    QToolBar* m_toolBar     = nullptr;
    QAction*  m_actDownload = nullptr;
    QAction*  m_actErase    = nullptr;
    QAction*  m_actOpen     = nullptr;
    QAction*  m_actSave     = nullptr;

    LogWindow* m_logWindow  = nullptr;

//static:
    static bool  kAskForConfgirmation;
};