#include "MainWindow.h"
#include <QApplication>
#include<QDesktopWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Globetools");
    QCoreApplication::setOrganizationDomain("Globetools.com");
    QCoreApplication::setApplicationName("COM_UART");

    QApplication a(argc, argv);
    qRegisterMetaType<MyQSerial::ButtonStatus>("ButtonStatus");//枚举类型
    MainWindow w;
    w.setWindowTitle("Greenworks");
    w.show();
    return a.exec();
}
