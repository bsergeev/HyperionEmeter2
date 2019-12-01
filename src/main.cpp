#ifdef _WIN32
#include <SDKDDKVer.h>
#ifndef WINVER                // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501        // Change this to the appropriate value to target other versions of Windows.
#endif
#ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501    // Change this to the appropriate value to target other versions of Windows.
#endif                        
#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif
#ifndef _WIN32_IE            // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600    // Change this to the appropriate value to target other versions of IE.
#endif
#endif

#include "MainWnd.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#if defined(_DEBUG) && defined(WIN32)
    _set_error_mode(_OUT_TO_MSGBOX);
    // Enabling memory checks below makes it real slow...
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
#endif

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("MileHighWings");
    QCoreApplication::setOrganizationDomain("milehighwings.com");
    QCoreApplication::setApplicationName("HyperionReader");

    MainWnd w;
    w.show();

    const auto result = a.exec();
    return result;
}
