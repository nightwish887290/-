#ifndef MYQSERIAL_H
#define MYQSERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QString>
#include <QByteArray>
#include "SettingsDialog.h"
#include "ParseMessage.h"
#include "string.h"
#include <QDebug>

class MyQSerial : public QObject
{
    Q_OBJECT
public:
    enum ButtonStatus{
        OpenSuccess,
        OpenFail,
        CloseSuccess,
        CloseFail,
    };
public:
    explicit MyQSerial(SettingsDialog *,QObject *parent = nullptr);
    ~MyQSerial();
    void closeThread();
    void process(QByteArray array);
    void Maxgetdata(QByteArray array);
    void Mingetdata(QByteArray array);
    void Basegetdata(QByteArray array);
    QThread *m_thread;

signals:
    void signalUpdateText(QString text);
    void signalUpdatestateText(QString text);
    void signalprocessgetdata(QStringList list);
    void signaltable(QString string);
    void signalMaxgetdata(QStringList list,int n);
    void signalMingetdata(QStringList list,int n);
    void signalBasegetdata(QStringList list);
    void signalButtonStatus(ButtonStatus status);
    void signalShowStatus(const QString &status);
    void signalHandleBootDataType(quint8 dataType);
    void signalpost(int i);


public slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const char *data, qint64 len);
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void destroyThread();

private:
    SettingsDialog  *settings;
    QSerialPort     *serial;
    ParseMessage    parseMessage;
};

#endif // MYQSERIAL_H
