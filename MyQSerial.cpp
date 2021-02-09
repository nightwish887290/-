#include "MyQSerial.h"
#include "ParseMessage.h"

#define Data_Src_FirmWareEnd            0xAF

MyQSerial::MyQSerial(SettingsDialog *settings,QObject *parent) : QObject(parent)
{
    this->settings = settings;
    m_thread = new QThread(this);
    serial = new QSerialPort();
    this->moveToThread(m_thread);
    serial->moveToThread(m_thread);
    m_thread->start();              //开启多线程
    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                this, &MyQSerial::handleError);
    connect(serial, &QSerialPort::readyRead, this, &MyQSerial::readData);
}

void MyQSerial::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();

    qDebug() << "MyQSerial::openSerialPort()";
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    emit signalUpdateText(tr("Connecting to %1").arg(p.name));
    emit signaltable(" ");

    if (serial->open(QIODevice::ReadWrite))
    {
        emit signalUpdateText("Connected success");
        emit signaltable(" ");
        emit signalShowStatus(tr("Connected to %1 : %2, %3, %4, %5, %6")
                              .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                              .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

        emit signalButtonStatus(ButtonStatus::OpenSuccess);
    }
    else
    {
        emit signalUpdateText("Connected fail");
        emit signaltable(" ");
        emit signalShowStatus(tr("Connected error"));
        emit signalButtonStatus(ButtonStatus::OpenFail);
    }
}

void MyQSerial::closeSerialPort()
{
    if (serial->isOpen())
    {
        serial->close();
    }
    emit signalShowStatus(tr("Disconnected"));
    emit signalUpdateText(tr("Disconnected to %1").arg(settings->settings().name));
    emit signaltable(" ");
    emit signalButtonStatus(ButtonStatus::CloseSuccess);
}

void MyQSerial::writeData(const char *data, qint64 len)
{
    qDebug()<<"write";
    serial->write(data,len);
}

void MyQSerial::readData()
{
    qDebug()<<"myQseial::readdate";
    QByteArray data = serial->readAll();
    serial->clear();
    parseMessage.ReceiveOneFrame(data);

    while(!parseMessage.getMessageBuf().isEmpty())
    {
        QByteArray dat;
        ParseMessage::Message message = parseMessage.getMessageBuf().dequeue();

        // 交给主线程处理接受到的数据类型
        if(quint8(Data_Src_FirmWareEnd) == message.messageSrc)
        {
            emit signalHandleBootDataType(message.messageRec);
        }

        dat.append(static_cast<char>(message.messageHeader[0]));
        dat.append(static_cast<char>(message.messageHeader[1]));
        dat.append(static_cast<char>(message.messageSrc));
        dat.append((static_cast<char>(message.messageRec)));
        dat.append(static_cast<char>(message.messageType));
        dat.append(static_cast<char>(message.messageLength));
        for(int i=0;i<message.messageLength;i++)
        {
            dat.append(static_cast<char>(message.messageData[i]));
        }

        dat.append(static_cast<char>(message.messageCrc));
        emit signalUpdateText(dat.toHex());
        if(char(0x00)==dat[4])
        {
            process(dat);
        }
        if(char(0x01)==dat[4])
        {
            Basegetdata(dat);
        }
        if(char(0x30)==dat[4])
        {
            Maxgetdata(dat);
        }
        if(char(0x31)==dat[4])
        {
            Mingetdata(dat);
        }
        if(char(0xE0)==dat[4])
        {
            emit signalpost(1);
        }
        if(char(0xE1)==dat[4])
        {
            emit signalpost(0);
        }
        if(char(0xE2)==dat[4])
        {
            emit signalpost(2);
            emit signalpost(1);
        }
        if(char(0xE3)==dat[4])
        {
            emit signalShowStatus(tr("Update successfully"));
        }
        if(char(0xE4)==dat[4])
        {
            emit signalShowStatus(tr("Updade failed"));
        }
    }
    data.clear();
}

void MyQSerial::Basegetdata(QByteArray array)
{
    QStringList List;
    QString Bseries;
    QString BParallel;
    QString Temp;
    QString BTemp;
    QString PVol;
    QString PVolEN;
    QString CVol,CVolEn,Dy,Dm,Dd;
    QString Cyclesum;
    QByteArray data;

    for(int i=0;i<array.length()-7;i++)
    {
        data[i]=array[i+6];
    }
    if(data.length()>=1)
    {
        Bseries=QString::number((unsigned char)data[0],10);
    }
    if(data.length()>=2)
    {
        BParallel=QString::number((unsigned char)data[1],10);
    }
    if(data.length()>=3)
    {
        Temp=QString::number((unsigned char)data[2],10);
    }
    if(data.length()>=4)
    {
        BTemp=QString::number((unsigned char)data[3],10);
    }
    if(data.length()>=6)
    {
        PVol=QString::number(((unsigned char)data[4]+((unsigned char)data[5]<<8))*0.01);
    }
    if(data.length()>=8)
    {
        PVolEN=QString::number(((unsigned char)data[6]+((unsigned char)data[7]<<8))*0.01);
    }
    if(data.length()>=10)
    {
        CVol=QString::number(((unsigned char)data[8]+((unsigned char)data[9]<<8))*0.01);
    }
    if(data.length()>=12)
    {
        CVolEn=QString::number(((unsigned char)data[10]+((unsigned char)data[11]<<8))*0.01);
    }
    if(data.length()>=14)
    {
        Dy=QString::number((unsigned char)data[12]+((unsigned char)data[13]<<8));
    }
    if(data.length()>=15)
    {
        Dm=QString::number((unsigned char)data[14],10);
    }
    if(data.length()>=16)
    {
        Dd=QString::number((unsigned char)data[15],10);
    }
    if(data.length()>=18)
    {
        Cyclesum=QString::number(((unsigned char)data[16]+((unsigned char)data[17]<<8)),10);
    }
    List <<Bseries
         <<BParallel
         <<Temp
         <<BTemp
         <<PVol
         <<PVolEN
         <<CVol
         <<CVolEn
         <<Dy
         <<Dm
         <<Dd
         <<Cyclesum;
    emit signalBasegetdata(List);

    QString str;
    str=tr("Series-number:%1,Parallel-number:%2,Environmental temperature:%3,Battery pack temperature:%4,No-load discharge port voltage:%5V，Load discharge port voltage:%6V,Open circuit charge port voltage:%7V,"
           "Closed circuit charge port voltage:%8V,Year of Production:%9,Month of Production:%10,Days of Production:%11,Number of cycles used:%12").arg(List.at(0),
                                                                List.at(1),List.at(2),List.at(3),List.at(4),
                                                                List.at(5),List.at(6),List.at(7),List.at(8),
                                                                           List.at(9),List.at(10),List.at(11));
    emit signaltable(str);
}

void MyQSerial::Maxgetdata(QByteArray array)
{
    QStringList List;
    QString celld;
    QString cellNum;
    QString V0,V1,V2,V3,V4,V5,V6,V7;
    QString V8,V9,V10,V11,V12,V13,V14;
    int n=0;
    QByteArray data;


    for(int i=0;i<array.length()-7;i++)
    {
        data[i]=array[i+6];
    }
    if (data.length()>=1)
    {
        celld=QString::number((unsigned char)data[0],10);
        n=data[0];
    }
    if(data.length()>=2)
    {
        cellNum=QString::number((unsigned char)data[1],10);
    }
    if(data.length()>=4)
    {
        V0=QString::number(((unsigned char)data[2]+((unsigned char)data[3]<<8))*0.01);
    }
    if(data.length()>=6)
    {
        V1=QString::number(((unsigned char)data[4]+((unsigned char)data[5]<<8))*0.01);
    }
    if(data.length()>=8)
    {
        V2=QString::number(((unsigned char)data[6]+((unsigned char)data[7]<<8))*0.01);
    }
    if(data.length()>=10)
    {
        V3=QString::number(((unsigned char)data[8]+((unsigned char)data[9]<<8))*0.01);
    }
    if(data.length()>=12)
    {
        V4=QString::number(((unsigned char)data[10]+((unsigned char)data[11]<<8))*0.01);
    }
    if(data.length()>=14)
    {
        V5=QString::number(((unsigned char)data[12]+((unsigned char)data[13]<<8))*0.01);
    }
    if(data.length()>=16)
    {
        V6=QString::number(((unsigned char)data[14]+((unsigned char)data[15]<<8))*0.01);
    }
    if(data.length()>=18)
    {
        V7=QString::number(((unsigned char)data[16]+((unsigned char)data[17]<<8))*0.01);
    }
    if(data.length()>=20)
    {
        V8=QString::number(((unsigned char)data[18]+((unsigned char)data[19]<<8))*0.01);
    }
    if(data.length()>=22)
    {
        V9=QString::number(((unsigned char)data[20]+((unsigned char)data[21]<<8))*0.01);
    }
    if(data.length()>=24)
    {
        V10=QString::number(((unsigned char)data[22]+((unsigned char)data[23]<<8))*0.01);
    }
    if(data.length()>=26)
    {
        V11=QString::number(((unsigned char)data[24]+((unsigned char)data[25]<<8))*0.01);
    }
    if(data.length()>=28)
    {
        V12=QString::number(((unsigned char)data[26]+((unsigned char)data[27]<<8))*0.01);
    }
    if(data.length()>=30)
    {
        V13=QString::number(((unsigned char)data[28]+((unsigned char)data[29]<<8))*0.01);
    }
    if(data.length()>=32)
    {
        V14=QString::number(((unsigned char)data[30]+((unsigned char)data[31]<<8))*0.01);
    }
    List <<celld
         <<cellNum
         <<V0
         <<V1
         <<V2
         <<V3
         <<V4
         <<V5
         <<V6
         <<V7
         <<V8
         <<V9
         <<V10
         <<V11
         <<V12
         <<V13
         <<V14;
    emit signalMaxgetdata(List,n);

    QString str;
    str=tr("Start cell number N:%1,Number of cells:%2,Historical maxmum voltage of the %18th cell:%3,Historical maxmum voltage of the %19th cell:%4,Historical maxmum voltage of the %20th cell:%5,Historical maxmum voltage of the %21th cell:%6,Historical maxmum voltage of the %22th cell:%7,"
           "Historical maxmum voltage of the %23th cell:%8,Historical maxmum voltage of the %24th cell:%9,Historical maxmum voltage of the %25th cell:%10,Historical maxmum voltage of the %26th cell:%11,Historical maxmum voltage of the %27th cell:%12,Historical maxmum voltage of the %28th cell:%13,"
           "Historical maxmum voltage of the %29th cell:%14,Historical maxmum voltage of the %30th cell:%15,Historical maxmum voltage of the %31th cell:%16,Historical maxmum voltage of the %32th cell:%17").arg(List.at(0),
                                                                                                                    List.at(1),List.at(2),List.at(3),List.at(4),
                                                                                                                    List.at(5),List.at(6),List.at(7),List.at(8),
                                                                                                                    List.at(9),List.at(10),List.at(11),List.at(12),
                                                                                                                    List.at(13),List.at(14),List.at(15),List.at(16)).arg(n).arg(n+1)
            .arg(n+2).arg(n+3).arg(n+4).arg(n+5).arg(n+6).arg(n+7).arg(n+8).arg(n+9).arg(n+10).arg(n+11).arg(n+12).arg(n+13).arg(n+14);
    emit signaltable(str);
}

void MyQSerial::process(QByteArray array)
{
    QStringList List;
    QString statestring;
    QByteArray data;

    for(int i=0;i<array.length()-7;i++)
    {
        data[i]=array[i+6];
    }
    if(data.length()>=1)
    {

        if(char(0x00)==data[0])
        {
            statestring="Leisure";
        }
        else if(0x01==data[0])
        {
            statestring="Diagnosing";
        }
        else if(0x02==data[0])
        {
            statestring="End";
        }
    }

    List << statestring;

    emit signalprocessgetdata(List);
    QString str;
    str=tr("Diagnostic working condition：%1").arg(List.at(0));
    emit signaltable(str);
}

void MyQSerial::Mingetdata(QByteArray array)
{
    QStringList List;
    QString celld;
    QString cellNum;
    QString V0,V1,V2,V3,V4,V5,V6,V7;
    QString V8,V9,V10,V11,V12,V13,V14;
    int n=1;
    QByteArray data;

    for(int i=0;i<array.length()-7;i++)
    {
        data[i]=array[i+6];
    }
    if (data.length()>=1)
    {
        celld=QString::number((unsigned char)data[0],10);
        n=data[0];
    }
    if(data.length()>=2)
    {
        cellNum=QString::number((unsigned char)data[1],10);
    }
    if(data.length()>=4)
    {
        V0=QString::number((((unsigned char)data[3]<<8)+(unsigned char)data[2])*0.01);
    }
    if(data.length()>=6)
    {
        V1=QString::number((((unsigned char)data[5]<<8)+(unsigned char)data[4])*0.01);
    }
    if(data.length()>=8)
    {
        V2=QString::number((((unsigned char)data[7]<<8)+(unsigned char)data[6])*0.01);
    }
    if(data.length()>=10)
    {
        V3=QString::number((((unsigned char)data[9]<<8)+(unsigned char)data[8])*0.01);
    }
    if(data.length()>=12)
    {
        V4=QString::number((((unsigned char)data[11]<<8)+(unsigned char)data[10])*0.01);
    }
    if(data.length()>=14)
    {
        V5=QString::number((((unsigned char)data[13]<<8)+(unsigned char)data[12])*0.01);
    }
    if(data.length()>=16)
    {
        V6=QString::number((((unsigned char)data[15]<<8)+(unsigned char)data[14])*0.01);
    }
    if(data.length()>=18)
    {
        V7=QString::number((((unsigned char)data[17]<<8)+(unsigned char)data[16])*0.01);
    }
    if(data.length()>=20)
    {
        V8=QString::number((((unsigned char)data[19]<<8)+(unsigned char)data[18])*0.01);
    }
    if(data.length()>=22)
    {
        V9=QString::number((((unsigned char)data[21]<<8)+(unsigned char)data[20])*0.01);
    }
    if(data.length()>=24)
    {
        V10=QString::number((((unsigned char)data[23]<<8)+(unsigned char)data[22])*0.01);
    }
    if(data.length()>=26)
    {
        V11=QString::number((((unsigned char)data[25]<<8)+(unsigned char)data[24])*0.01);
    }
    if(data.length()>=28)
    {
        V12=QString::number((((unsigned char)data[27]<<8)+(unsigned char)data[26])*0.01);
    }
    if(data.length()>=30)
    {
        V13=QString::number((((unsigned char)data[29]<<8)+(unsigned char)data[28])*0.01);
    }
    if(data.length()>=32)
    {
        V14=QString::number((((unsigned char)data[31]<<8)+(unsigned char)data[30])*0.01);
    }
    List <<celld
         <<cellNum
         <<V0 <<V1 <<V2 <<V3 <<V4 <<V5 <<V6 <<V7 << V8 <<V9
         <<V10 <<V11 <<V12 <<V13 <<V14;
    emit signalMingetdata(List,n);

    QString str;
    str=tr("Start cell number N:%1,Number of cells:%2,Historical minimum voltage of the %18th cell::%3,Historical minimum voltage of the %19th cell::%4,Historical minimum voltage of the %20th cell::%5,Historical minimum voltage of the %21th cell::%6,"
           "Historical minimum voltage of the %22th cell:%7,"
           "Historical minimum voltage of the %23th cell::%8,Historical minimum voltage of the %24th cell::%9,Historical minimum voltage of the %25th cell:%10,Historical minimum voltage of the %26th cell:%11,Historical minimum voltage of the %27th cell:%12,Historical minimum voltage of the %28th cell:%13,"
           "Historical minimum voltage of the %29th cell:%14,Historical minimum voltage of the %30th cell:%15,Historical minimum voltage of the %31th cell:%16,Historical minimum voltage of the %32th cell:%17").arg(List.at(0),
                                                                                                                    List.at(1),List.at(2),List.at(3),List.at(4),
                                                                                                                    List.at(5),List.at(6),List.at(7),List.at(8),
                                                                                                                    List.at(9),List.at(10),List.at(11),List.at(12),
                                                                                                                    List.at(13),List.at(14),List.at(15),List.at(16)).arg(n).arg(n+1)
            .arg(n+2).arg(n+3).arg(n+4).arg(n+5).arg(n+6).arg(n+7).arg(n+8).arg(n+9).arg(n+10).arg(n+11).arg(n+12).arg(n+13).arg(n+14);
    emit signaltable(str);
}

void MyQSerial::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        closeSerialPort();
        emit signalButtonStatus(ButtonStatus::CloseSuccess);
    }
}

void MyQSerial::destroyThread()
{
    qDebug() << "Enter MyQSerial::~destroyThread()" << QThread::currentThreadId();
    closeSerialPort();
    m_thread->quit();
    qDebug() << "Exit MyQSerial::~destroyThread()";
}

MyQSerial::~MyQSerial()
{
    qDebug() << "Enter MyQSerial::~MyQSerial()";
    delete m_thread;
    delete serial;
    qDebug() << "Exit MyQSerial::~MyQSerial()";
}

