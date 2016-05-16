#pragma once

#include "HypReader.h"

#include <QMainWindow>
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

private:
    void DownloadFromDevice();
    void EraseDevice ();
    void LoadFromFile();
    void SaveToFile  ();

    void updateMenus();

    void DisplayMessageFromReader(const QString& msg);

    // These get called from DeviceLink worker thread.
    // They emit the data to be processed on UI thread.
    void DisplayMessage(const QString& msg);

    void ReadSettings();
    void WriteSettings();

//data:
    std::unique_ptr<HypReader> m_reader;

    QMdiArea* m_mdiArea = nullptr;
    QToolBar* m_toolBar = nullptr;
    QAction*  m_actDownload = nullptr;
    QAction*  m_actErase = nullptr;
    QAction*  m_actOpen  = nullptr;
    QAction*  m_actSave  = nullptr;

    LogWindow* m_logWindow;

//static:
    static bool  kAskForConfgirmation;
};