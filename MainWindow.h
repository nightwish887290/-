#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include "MyQSerial.h"
#include <QTimer>
#include <QProgressDialog>


class QLabel;
class SettingsDialog;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool event(QEvent *e);
    void fillSendBuf(int i);
    bool save();      // 保存操作
    bool saveAs();    // 另存为操作
    bool saveFile(const QString &fileName); // 保存文件
    unsigned char crc_high_first(unsigned char *ptr, unsigned char len);
    ~MainWindow();

signals:
    void signalOpenSerialPort();
    void signalCloseSerialPort();
    void signalDestroy();
    void signalParseFile();
    void signalWriteSerialPort(const char *data, qint64 len);

private slots:
    void onClearTextEdit();
    void appendTable1(QString text);
    void appendTable2(QString str);
    void appendprocessLabel(QStringList list);
    void appendMaxLabel(QStringList list,int a);
    void appendMinLabel(QStringList list,int a);
    void appendBaseLabel(QStringList list);
    void onOpenSerialPort();
    void onCloseSerialPort();
    void onButtonStatus(MyQSerial::ButtonStatus status);
    void onShowStatus(const QString &message);
    void timerTransDate(int i);

    void on_send_clicked();



    void on_pushButton_clicked();

    void on_actionSave_triggered();

    void on_actionUpdate_File_triggered();

private:
    void initActionsConnections();

private:
    Ui::MainWindow  *ui;
    SettingsDialog  *settings;
    MyQSerial       *uartPort;
    QLabel          *status;
    quint32         dataCounter;
    QByteArray      sendBuf;
    bool            readyFlag;
    QProgressDialog *progress;
    QTimer* timer;
    int ulNum;
    QString FileName;
    int num;
    int binfileseek;
};

#endif // MAINWINDOW_H
