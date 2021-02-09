#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QEvent>
#include <QSettings>
#include "SettingsDialog.h"
#include <QFileDialog>
#include <QElapsedTimer>
#include <QPushButton>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <ActiveQt/QAxObject>
#include <QDesktopServices>
#include <QUrl>
#include <ParseMessage.h>

#define DataType_App_ProgramData        0x00

#define App                             0xA7
#define ARDK                            0xA9



MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dataCounter(0),readyFlag(false),progress(nullptr)
{

    ui->setupUi(this);
    settings = new SettingsDialog();
    uartPort = new MyQSerial(settings);

    QStringList list;
    list<<"data"<<"Message";
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->setHorizontalHeaderLabels(list);
    ui->tableWidget->horizontalScrollBar();

    // 状态栏显示
    status = new QLabel;
    ui->statusBar->addWidget(status);

    // 三个按钮使能设置
    ui->action_Connect->setEnabled(true);
    ui->action_Disconnect->setEnabled(false);
    ui->action_Settings->setEnabled(true);

    //连接主线程和串口线程信号和槽
    connect(this,&MainWindow::signalOpenSerialPort,uartPort,&MyQSerial::openSerialPort);
    connect(this,&MainWindow::signalCloseSerialPort,uartPort,&MyQSerial::closeSerialPort);
    connect(this,&MainWindow::signalDestroy,uartPort,&MyQSerial::destroyThread,Qt::BlockingQueuedConnection);
    connect(this,&MainWindow::signalWriteSerialPort,uartPort,&MyQSerial::writeData);
    connect(uartPort,&MyQSerial::signalUpdateText,this,&MainWindow::appendTable1);
    connect(uartPort,&MyQSerial::signaltable,this,&MainWindow::appendTable2);
    connect(uartPort,&MyQSerial::signalprocessgetdata,this,&MainWindow::appendprocessLabel);
    connect(uartPort,&MyQSerial::signalMaxgetdata,this,&MainWindow::appendMaxLabel);
    connect(uartPort,&MyQSerial::signalBasegetdata,this,&MainWindow::appendBaseLabel);
    connect(uartPort,&MyQSerial::signalMingetdata,this,&MainWindow::appendMinLabel);
    connect(uartPort,&MyQSerial::signalButtonStatus,this,&MainWindow::onButtonStatus);
    connect(uartPort,&MyQSerial::signalShowStatus,this,&MainWindow::onShowStatus);

    initActionsConnections();
    uartPort->readData();


    connect(uartPort,&MyQSerial::signalpost,this,&MainWindow::timerTransDate);
    ulNum = 0;

    qDebug() << "MainWindow::MainWindow():" << QThread::currentThreadId();
}

bool MainWindow::event(QEvent *e)
{
    if(e->type() == QEvent::Close)
    {
        emit signalDestroy();
    }
    return QMainWindow::event(e);
}

void MainWindow::initActionsConnections()
{
    connect(ui->action_Connect, &QAction::triggered, this, &MainWindow::onOpenSerialPort);
    connect(ui->action_Disconnect, &QAction::triggered, this, &MainWindow::onCloseSerialPort);
    connect(ui->action_Settings, &QAction::triggered, settings, &MainWindow::show);
    connect(ui->action_Settings, &QAction::triggered, settings, &SettingsDialog::updatePortNum);
    connect(ui->action_Clear,&QAction::triggered,this,&MainWindow::onClearTextEdit);
}
void MainWindow::appendTable1(QString text)
{
    QTableWidgetItem *item;
    item=new QTableWidgetItem(text);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,0,item);
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    ui->tableWidget->scrollToBottom(); //滚动到最后
}

void MainWindow::appendTable2(QString str)
{
    QTableWidgetItem *item;
    item=new QTableWidgetItem(str);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-2,1,item);
}

void MainWindow::appendprocessLabel(QStringList list)
{

    ui->processlabel->setText(tr("Diagnostic working condition: %1").arg(list.count() >=1 ? list.at(0):tr(" ")));

}

void MainWindow::appendBaseLabel(QStringList list)
{
    ui->BSerieslabel->setText(tr("Series-number:%1").arg(list.count()>=1?list.at(0):tr(" ")));
    ui->BParallellabel->setText(tr("Parallel-number:%1").arg(list.count()>=2?list.at(1):tr(" ")));
    ui->Templabel->setText(tr("Environmental temperature:%1").arg(list.count()>=3?list.at(2):tr(" ")));
    ui->BTemplabel->setText(tr("Battery pack temperature:%1").arg(list.count()>=4?list.at(3):tr(" ")));
    ui->PVollabel->setText(tr("No-load discharge port voltage:%1V").arg(list.count()>=5?list.at(4):tr(" ")));
    ui->PVolEnlabel->setText(tr("Load discharge port voltage:%1V").arg(list.count()>=6?list.at(5):tr(" ")));
    ui->CVollabel->setText(tr("Open circuit charge port voltage:%1V").arg(list.count()>=7?list.at(6):tr(" ")));
    ui->CVolEnlabel->setText(tr("Closed circuit charge port voltage:%1V").arg(list.count()>=8?list.at(7):tr(" ")));
    ui->Dylabel->setText(tr("Year of Production:%1").arg(list.count()>=9?list.at(8):tr(" ")));
    ui->Dmlabel->setText(tr("Month of Production:%1").arg(list.count()>=10?list.at(9):tr(" ")));
    ui->Ddlabel->setText(tr("Days of Production:%1").arg(list.count()>=11?list.at(10):tr(" ")));
    ui->CycleNumlabel->setText(tr("Number of cycles used:%1").arg(list.count()>=12?list.at(11):tr(" ")));
}

void MainWindow::appendMaxLabel(QStringList list,int a)
{
    ui->Celldlabel->setText(tr("Start cell number N:%1").arg(list.count()>=1?list.at(0):tr(" ")));
    ui->Cellnumlabel->setText(tr("Number of cells:%1").arg(list.count()>=2?list.at(1):tr(" ")));
    ui->V0label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=3?list.at(2):tr(" ")).arg(a));
    ui->V1label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=4?list.at(3):tr(" ")).arg(a+1));
    ui->V2label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=5?list.at(4):tr(" ")).arg(a+2));
    ui->V3label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=6?list.at(5):tr(" ")).arg(a+3));
    ui->V4label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=7?list.at(6):tr(" ")).arg(a+4));
    ui->V5label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=8?list.at(7):tr(" ")).arg(a+5));
    ui->V6label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=9?list.at(8):tr(" ")).arg(a+6));
    ui->V7label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=10?list.at(9):tr(" ")).arg(a+7));
    ui->V8label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=11?list.at(10):tr(" ")).arg(a+8));
    ui->V9label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=12?list.at(11):tr(" ")).arg(a+9));
    ui->V10label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=13?list.at(12):tr(" ")).arg(a+10));
    ui->V11label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=14?list.at(13):tr(" ")).arg(a+11));
    ui->V12label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=15?list.at(14):tr(" ")).arg(a+12));
    ui->V13label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=16?list.at(15):tr(" ")).arg(a+13));
    ui->V14label->setText(tr("Historical maxmum voltage of the %2th cell:%1V").arg(list.count()>=17?list.at(16):tr(" ")).arg(a+14));
}

void MainWindow::appendMinLabel(QStringList list,int a)
{
    ui->celldlabel->setText(tr("Start cell number N:%1").arg(list.count()>=1?list.at(0):tr(" ")));
    ui->cellnumlabel->setText(tr("Number of cells:%1").arg(list.count()>=2?list.at(1):tr(" ")));
    ui->MinV0label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=3?list.at(2):tr(" ")).arg(a));
    ui->MinV1label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=4?list.at(3):tr(" ")).arg(a+1));
    ui->MinV2label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=5?list.at(4):tr(" ")).arg(a+2));
    ui->MinV3label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=6?list.at(5):tr(" ")).arg(a+3));
    ui->MinV4label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=7?list.at(6):tr(" ")).arg(a+4));
    ui->MinV5label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=8?list.at(7):tr(" ")).arg(a+5));
    ui->MinV6label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=9?list.at(8):tr(" ")).arg(a+6));
    ui->MinV7label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=10?list.at(9):tr(" ")).arg(a+7));
    ui->MinV8label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=11?list.at(10):tr(" ")).arg(a+8));
    ui->MinV9label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=12?list.at(11):tr(" ")).arg(a+9));
    ui->MinV10label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=13?list.at(12):tr(" ")).arg(a+10));
    ui->MinV11label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=14?list.at(13):tr(" ")).arg(a+11));
    ui->MinV12label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=15?list.at(14):tr(" ")).arg(a+12));
    ui->MinV13label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=16?list.at(15):tr(" ")).arg(a+13));
    ui->MinV14label->setText(tr("Historical minimum voltage of the %2th cell:%1V").arg(list.count()>=17?list.at(16):tr(" ")).arg(a+14));
}

void MainWindow::onButtonStatus(MyQSerial::ButtonStatus status)
{
    if(MyQSerial::OpenSuccess == status)
    {
        ui->action_Connect->setEnabled(false);
        ui->action_Disconnect->setEnabled(true);
        ui->action_Settings->setEnabled(true);
    }
    else if(MyQSerial::OpenFail == status)
    {
        ui->action_Connect->setEnabled(true);
        ui->action_Disconnect->setEnabled(false);
        ui->action_Settings->setEnabled(true);
    }
    else if(MyQSerial::CloseSuccess == status)
    {
        ui->action_Connect->setEnabled(true);
        ui->action_Disconnect->setEnabled(false);
        ui->action_Settings->setEnabled(true);
    }
    else if(MyQSerial::CloseFail == status)
    {
        ui->action_Connect->setEnabled(false);
        ui->action_Disconnect->setEnabled(true);
        ui->action_Settings->setEnabled(true);
    }
}

void MainWindow::onClearTextEdit()
{
    for(int row =  ui->tableWidget->rowCount() - 1;row >= 0; row--)
    {
        ui->tableWidget->removeRow(row);
    }
    ui->tableWidget->setRowCount(1);
}

void MainWindow::onShowStatus(const QString &message)
{
    status->setText(message);
}

void MainWindow::onOpenSerialPort()
{
    emit signalOpenSerialPort();
}

void MainWindow::onCloseSerialPort()
{
    emit signalCloseSerialPort();
}

void  MainWindow::fillSendBuf(int i)
{
    sendBuf.clear();
    sendBuf.append(HEADER_0);
    sendBuf.append(HEADER_1);
    sendBuf.append(App);
    sendBuf.append(ARDK);
    if(0==i)
    {
        sendBuf.append(char(0x00));
        sendBuf.append(char(0x00));
        sendBuf.append(0x57);
    }
    if (1==i)
    {
        sendBuf.append(0x01);
        sendBuf.append(char(0x00));
        sendBuf.append(0x93);
    }
    if(2==i)
    {
        sendBuf.append(0x30);
        sendBuf.append(char(0x00));
        sendBuf.append(0x7A);
    }
    if(3==i)
    {
        sendBuf.append(0x31);
        sendBuf.append(char(0x00));
        sendBuf.append(0xBE);
    }
    if(4==i)
    {
        sendBuf.append(0xE1);
        sendBuf.append(0x06);
        sendBuf.append(0x01);
        sendBuf.append(0x02);
        sendBuf.append(0x03);
        sendBuf.append(0x04);
        sendBuf.append(0x05);
        sendBuf.append(0x06);
        sendBuf.append(0x95);
    }

}

void MainWindow::timerTransDate(int i)
{
    //static int num=0;
    //static int binfileseek = 0;
    QFile *binFile = new QFile(FileName);

    QFileInfo *pcsfileInfo = new QFileInfo(FileName);
    qint64 binSize = pcsfileInfo->size ();
    binFile->open (QIODevice::ReadOnly);
    binFile->seek (ulNum * 1024);

    QDataStream in(binFile);
    char * binByte = new char[binSize];

    in.readRawData (binByte, binSize);    //读出文件到缓存



    qint16 temp = binSize -32*num;       //剩余待传数据
    if(1==i)
    {
        sendBuf.clear();
        sendBuf.append(HEADER_0);
        sendBuf.append(HEADER_1);
        if(temp>=32)
        {
            sendBuf.append(App);
            sendBuf.append(ARDK);
            sendBuf.append(0xE0);
            sendBuf.append(0x20);
            sendBuf.append(binByte+binfileseek,32);
            QByteArray  data;
            for(int i=0;i<36;i++)
            {
                data[i]=sendBuf[i+2];
            }
            quint8 a;
            ParseMessage p;
            a=p.Crc8_Calc(0,reinterpret_cast<quint8*>(data.data()),36);
            sendBuf.append(a);
            emit signalWriteSerialPort(sendBuf,sendBuf.length());
            num++;
            binfileseek += 32;
        }
        if(temp>0&&temp<32)
        {
            sendBuf.append(App);
            sendBuf.append(ARDK);
            sendBuf.append(0xE0);
            sendBuf.append(temp);
            sendBuf.append(binByte+binfileseek,temp);
            QByteArray  data;
            for(int i=0;i<temp+4;i++)
            {
                data[i]=sendBuf[i+2];
            }
            quint8 a;
            ParseMessage p;
            a=p.Crc8_Calc(0,reinterpret_cast<quint8*>(data.data()),temp+4);
            sendBuf.append(a);
            emit signalWriteSerialPort(sendBuf,sendBuf.length());
            num++;
            binfileseek += 32;
        }
        if(temp<=0)
        {
            sendBuf.clear();
            sendBuf.append(HEADER_0);
            sendBuf.append(HEADER_1);
            sendBuf.append(App);
            sendBuf.append(ARDK);
            sendBuf.append(0xE2);
            sendBuf.append(char(0x00));
            sendBuf.append(0xB3);
            emit signalWriteSerialPort(sendBuf,sendBuf.length());
         }
    }
    if (0==i)
    {
        emit signalWriteSerialPort(sendBuf,sendBuf.length());
    }
    if(2==i)
    {
        num=0;
        binfileseek=0;
    }
    delete binByte;

}

MainWindow::~MainWindow()
{
    qDebug() << "Enter MainWindow::~MainWindow()";

    while(uartPort->m_thread->isRunning())
    {
        qDebug() << "Before while(!uartPort->m_thread->isFinished())";
    }

    delete uartPort;
    delete settings;
    delete ui;

    qDebug() << "MainWindow::~MainWindow()";
}

void MainWindow::on_send_clicked()
{

    int index = ui->comboBox->currentIndex();
    fillSendBuf(index);
    emit signalWriteSerialPort(sendBuf,sendBuf.length());

}

void MainWindow::on_pushButton_clicked()
{
    if(FileName.isEmpty())
    {
        QMessageBox::information(this,"Error Message", "Please Select a bin File");
        return;
    }
    else
    {
        fillSendBuf(4);
        emit signalWriteSerialPort(sendBuf,sendBuf.length());
    }
    binfileseek=0;
    num=0;
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save",QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),"Excel 文件(*.xls *.xlsx)");
         if (fileName!="")
         {
             QAxObject *excel = new QAxObject;
             if (excel->setControl("Excel.Application")) //连接Excel控件
             {
                 excel->dynamicCall("SetVisible (bool Visible)","false");//不显示窗体
                 excel->setProperty("DisplayAlerts", false);//不显示任何警告信息。如果为true那么在关闭是会出现类似“文件已修改，是否保存”的提示
                 QAxObject *workbooks = excel->querySubObject("WorkBooks");//获取工作簿集合
                 workbooks->dynamicCall("Add");//新建一个工作簿
                 QAxObject *workbook = excel->querySubObject("ActiveWorkBook");//获取当前工作簿
                 QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);

                 int i,j;
                 //QTablewidget 获取数据的列数
                 int colcount=ui->tableWidget->columnCount();
                  //QTablewidget 获取数据的行数
                 int rowscount=ui->tableWidget->rowCount();

                 QAxObject *cell,*col;
                 QAxObject *range ;
                 //列标题
                 for(i=0;i<colcount;i++)
                 {
                     QString columnName;
                     columnName.append(QChar(i + 'A'));
                     columnName.append(":");
                     columnName.append(QChar(i + 'A'));
                     col = worksheet->querySubObject("Columns(const QString&)", columnName);
                     col->setProperty("ColumnWidth", ui->tableWidget->columnWidth(i)/6);
                     cell=worksheet->querySubObject("Cells(int,int)", 1, i+1);
                     //QTableWidget 获取表格头部文字信息
                     columnName=ui->tableWidget->horizontalHeaderItem(i)->text();
                     cell->dynamicCall("SetValue(const QString&)", columnName);
                 }

                //数据区

                //QTableWidget 获取表格数据部分
                 for(i=0;i<rowscount;i++){
                     for (j=0;j<colcount;j++)
                     {
                         worksheet->querySubObject("Cells(int,int)", i+2, j+1)->dynamicCall("SetValue(const QString&)", ui->tableWidget->item(i,j)?ui->tableWidget->item(i,j)->text():"");
                     }
                 }

                 //画框线
                 QString lrange;
                 lrange.append("A1:");
                 lrange.append(colcount - 1 + 'A');
                 lrange.append(QString::number(ui->tableWidget->rowCount() + 2));
                 range = worksheet->querySubObject("Range(const QString&)", lrange);

                 //调整数据区行高
                 QString rowsName;
                 rowsName.append("1:");
                 rowsName.append(QString::number(ui->tableWidget->rowCount() + 2));
                 range = worksheet->querySubObject("Range(const QString&)", rowsName);

                 workbook->dynamicCall("SaveAs(const QString&)",QDir::toNativeSeparators(fileName));//保存至fileName
                 workbook->dynamicCall("Close()");//关闭工作簿
                 excel->dynamicCall("Quit()");//关闭excel
                 delete excel;
                 excel=NULL;
                 if (QMessageBox::question(NULL,"Finish","The file is exported, do you want to open it now?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
                 {
                     QDesktopServices::openUrl(QUrl("file:///" + QDir::toNativeSeparators(fileName)));
                 }
             }
             else
             {
                 QMessageBox::warning(NULL,"Error","Failed to create Excel object, please install Microsoft Excel.",QMessageBox::Apply);
             }
         }
}


void MainWindow::on_actionUpdate_File_triggered()
{
    FileName = QFileDialog::getOpenFileName(this,"Choose file",QDir::currentPath());
   /*if(FileName.isEmpty())
    {
        QMessageBox::information(this,"Error Message", "Please Select a Text File");
        return;
    }
    */
}
