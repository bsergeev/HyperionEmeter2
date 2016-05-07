#include "HypReader.h"
#include <QtWidgets/QApplication>

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

    AtxReader w;
    w.show();
    return a.exec();
}
