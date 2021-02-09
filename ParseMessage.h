#ifndef PARSEMESSAGE_H
#define PARSEMESSAGE_H

#include <QObject>
#include <QQueue>

#define HEADER_LENGTH   0x02

#define HEADER_0        0xAA
#define HEADER_1        0x55


class ParseMessage : public QObject
{
    Q_OBJECT
public:

    struct Message
    {
        quint8      messageHeader[HEADER_LENGTH];
        quint8      messageSrc;
        quint8      messageRec;
        quint8      messageType;
        quint8      messageLength;
        quint8      messageData[32];
        quint8      messageCrc;
    };

    enum StateMachine
    {
        stateMachineHeader_0 = 0,
        stateMachineHeader_1 = 1,
        stateMachineSrc,
        StateMachineRec,
        stateMachineType,
        stateMachineLength,
        stateMachineData,
        stateMachineCrc,
    };

    struct RecvCnter
    {
        StateMachine    stateMachine;   //状态机
        quint8          messageCrc;        //根据接收到的数据计算出来的SUMCRC
        quint8          messageLength;      //记录接收到的数据部分的个数
    };

    explicit ParseMessage(QObject *parent = nullptr);
    void ReceiveOneFrame(QByteArray &data);
    quint8 Crc8_Calc(quint8 crc, quint8 *message, quint32 length);
    QQueue<Message> &getMessageBuf(){return messageBuf;}

signals:

public slots:

private:
    RecvCnter       rcvCnter;
    Message         messageSend;
    Message         messageRcv;
    QQueue<Message> messageBuf;
};

#endif // PARSEMESSAGE_H
