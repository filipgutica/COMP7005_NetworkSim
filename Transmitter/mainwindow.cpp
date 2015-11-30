#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "listenthread.h"
#include "sendthread.h"
#include "globals.h"
#include <Windows.h>

QMutex mutex;
QSemaphore sem1;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listView->setEditTriggers(QListView::NoEditTriggers);
    loadFiles();
    updateFileList();

    allPacketsAckd = true;
    currentPacketWindow = new QVector<packet>();
    receivedControlPackets = new QVector<packet>();

    tx_socket = new QUdpSocket(this);
    tx_socket->bind(QHostAddress::AnyIPv4, TRANSMIT_PORT);
    connect(tx_socket, SIGNAL(readyRead()), this, SLOT(readtxDatagrams()));

    // Timer for ack timeouts
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),this, SLOT(timeoutEvent()));


#ifndef THREADED
    qDebug() << "Not Threaded";
    rx_socket = new QUdpSocket(this);
    rx_socket->bind(QHostAddress::AnyIPv4, RECEIVER_PORT);
    connect(rx_socket, SIGNAL(readyRead()), this, SLOT(readrxDatagrams()));

#endif

#ifdef THREADED
    qDebug() << "Threaded";
    ListenThread *thrd = new ListenThread(this);
    thrd->start();
#endif

    connect(this, SIGNAL(valueChanged(QString)), ui->log, SLOT(append(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateFileList()
{
    //Add to the list view
    ui->listView->setModel(new QStringListModel(_fileList));
}

void MainWindow::AppendToLog(QString s)
{
    emit valueChanged(s);
}


void MainWindow::loadFiles()
{
    QDirIterator dirIter("../Files", QDirIterator::Subdirectories);
    QString curFile;

    while (dirIter.hasNext())
    {
        dirIter.next();
        if (QFileInfo(dirIter.filePath()).isFile())
        {

            curFile = dirIter.filePath();
          //  qDebug() << curFile;
            _fileList.push_back(curFile);

        }
    }
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    SendThread *sendThrd = new SendThread(this);

    sendThrd->setData(index.data().toString());

    sendThrd->start();

    //timer->start(500);

    /*
    char temp[DATA_SIZE];
    int i =0;
    int seqNum = 0;
    packet dgram;
    QFile file(index.data().toString());

    //open the file for reading
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "cannot find file";
        return;
    }

    timer->start(500);

    while ((file.read(temp, DATA_SIZE)))
    {
        if (allPacketsAckd)
        {
            float window = file.size()/DATA_SIZE;
            BuildPacket(dgram, 0, seqNum, (int)window, DATA_PACKET, RECEIVER_PORT, temp, (char*)RECV_ADDR);

            if(currentPacketWindow->size() <= WINDOW_SIZE)
                currentPacketWindow->push_back(dgram);

            // Write the datagram
            Sleep(500);
            for (int j = 0; j < currentPacketWindow->size(); j++)
            {
                WriteUDP(currentPacketWindow->at(j));
            }

            currentPacketWindow->clear();

            i += DATA_SIZE;
            seqNum++;
            file.seek(i);
            memset(temp, 0, sizeof(temp));
        }
    }
    */
}

void MainWindow::readtxDatagrams()
{
    packet packet;

    while (tx_socket->hasPendingDatagrams())
    {
        tx_socket->readDatagram((char*)&packet, sizeof(packet));
        ProcessPacket(packet);
    }
}

void MainWindow::readrxDatagrams()
{
    packet packet;

    while (rx_socket->hasPendingDatagrams())
    {
        rx_socket->readDatagram((char*)&packet, sizeof(packet));
        ProcessPacket(packet);
    }
}

void MainWindow::WriteUDP(packet p)
{
    tx_socket->writeDatagram( (char*)&p, sizeof(p), QHostAddress(NETWORK_ADDR), NETWORK_PORT);
}


void MainWindow::ProcessPacket(packet p)
{
    //PrintPacketInfo(p);

    switch (p.PacketType)
    {
        case CONTROL_PACKET:
            sem1.acquire();
            PrintPacketInfo(p);
            sem2.release();

            break;
        case DATA_PACKET:
            packet dgram;

            BuildPacket(dgram, p.SeqNum + 1, p.SeqNum, 0, CONTROL_PACKET, TRANSMIT_PORT, (char*)"ACK", (char*)TRANSMIT_ADDR);
            WriteUDP(dgram);
            break;
        default:
            break;
    }
}

 void MainWindow::BuildPacket(packet &p, int ack, int seq, int win, int type, int destPort, char* data, char* destAddr)
 {
     p.AckNum = ack;
     p.PacketType = type;
     sprintf(p.data, "%s", data);
     p.SeqNum = seq;
     p.WindowSize = win;
     sprintf(p.dest_addr, "%s", destAddr);
     p.dest_port = destPort;
 }


 void MainWindow::PrintPacketInfo(packet p)
 {
     QString pType;

     switch (p.PacketType)
     {
         case CONTROL_PACKET:
             pType = "Control";
             break;
         case DATA_PACKET:
             pType = "Data";
             break;
         default:
             pType = "Unknown";
             break;
     }

     QString packetInfo = QString("Packet Type: %1\n"
                                  "Sequence: %2\n"
                                  "WindowSize: %3\n"
                                  "AckNum: %4\n"
                                  "Destination addr: %5\n"
                                  "Destination port: %6\n")
             .arg(pType,
                  QString::number(p.SeqNum),
                  QString::number(p.WindowSize),
                  QString::number(p.AckNum),
                  p.dest_addr,
                  QString::number(p.dest_port));

     AppendToLog(packetInfo);
    // AppendToLog(p.data);
 }

 void MainWindow::timeoutEvent()
 {
    AppendToLog(QString("Timeout!!!"));
 }

