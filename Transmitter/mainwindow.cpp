#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "listenthread.h"
#include "sendthread.h"
#include "globals.h"
#include <Windows.h>

QMutex mutex;
QSemaphore sem1;
QSemaphore sem2;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listView->setEditTriggers(QListView::NoEditTriggers);
    loadFiles();
    updateFileList();

    QSettings settings(QString("../config.ini"), QSettings::IniFormat);
    recv_addr = settings.value("settings/receiver_address", "127.0.0.1").toString();
    network_addr = settings.value("settings/network_address", "127.0.0.1").toString();
    transmit_addr = settings.value("settings/transmit_address", "127.0.0.1").toString();
    timeout = settings.value("settings/timeout", "500").toInt();
    window_size = settings.value("settings/window_size", "5").toInt();
    max_retransmissions = settings.value("settings/max_retransmissions", "4").toInt();
    network_port = settings.value("settings/network_port", "7003").toInt();
    receiver_port = settings.value("settings/receiver_port", "7001").toInt();
    transmit_port = settings.value("settings/transmit_port", "7002").toInt();

    sem2.release(window_size+1);

    reTransmitCount = 0;
    allPacketsAckd = true;
    currentPacketWindow = new QVector<packet>();
    receivedControlPackets = new QVector<packet>();
    retransmitPackets = new QVector<packet>();
    receivedDataPackets = new QVector<packet>();
    lastPacket = false;

    tx_socket = new QUdpSocket(this);
    tx_socket->bind(QHostAddress::AnyIPv4, transmit_port);
    connect(tx_socket, SIGNAL(readyRead()), this, SLOT(readtxDatagrams()));

    // Timer for ack timeouts
    timer = new QTimer(this);
    EOTAckTimer = new QTimer(this);
    connect(EOTAckTimer, SIGNAL(timeout()), this, SLOT(eot_ack_timeout()));
    connect(timer, SIGNAL(timeout()),this, SLOT(timeoutEvent()));

    ListenThread *thrd = new ListenThread(this);
    thrd->start();

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

    lastPacket = false;

    sendThrd->setData(index.data().toString(), recv_addr);

    sendThrd->start();

    timer->start(timeout);

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

void MainWindow::WriteUDP(packet p)
{
    tx_socket->writeDatagram( (char*)&p, sizeof(p), QHostAddress(network_addr), network_port);
}


void MainWindow::ProcessPacket(packet p)
{
    switch (p.PacketType)
    {
        case CONTROL_PACKET:


            timer->start(timeout);
            receivedControlPackets->push_back(p);
            qDebug() << receivedControlPackets->size();
            PrintPacketInfo(p);
            if (receivedControlPackets->size() >= window_size || lastPacket)
            {

                receivedControlPackets->clear();
                currentPacketWindow->clear();
                sem2.release(window_size+1);
            }

            break;
        case DATA_PACKET:
            packet dgram;

            if (!isDuplicatePacket(p))
            {
                receivedDataPackets->push_back(p);

                PrintPacketInfo(p);
                BuildPacket(dgram, p.SeqNum, p.SeqNum, 0, CONTROL_PACKET, transmit_port, (char*)"ACK", (char*)transmit_addr.constData());
                WriteUDP(dgram);
            }
            break;
        case EOT_PACKET:
            packet eot_packet;
            AppendToLog("EOT");
            receivedDataPackets->clear();
            BuildPacket(eot_packet, p.SeqNum, p.SeqNum, 0, EOT_ACK_PACKET, transmit_port, (char*)"ACK", (char*)transmit_addr.constData());
            WriteUDP(eot_packet);
            break;
        case EOT_ACK_PACKET:
            AppendToLog("EOT-ACK");
            currentPacketWindow->clear();
            receivedControlPackets->clear();
            reTransmitCount = 0;
            lastPacket = false;
            sem2.release(window_size+1);
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
    if (reTransmitCount < max_retransmissions && !lastPacket)
    {
        reTransmitCount ++;
        for (int i = 0; i < currentPacketWindow->size(); i++)
        {
            for (int j = 0; j < receivedControlPackets->size(); j++)
            {
                //qDebug() << receivedControlPackets->at(j).AckNum << "  |  " << currentPacketWindow->at(i).SeqNum;
                if (receivedControlPackets->at(j).AckNum == currentPacketWindow->at(i).SeqNum)
                {
                    currentPacketWindow->remove(i);

                }
            }
        }

        retransmitPackets = currentPacketWindow;

        for (int i = 0; i < retransmitPackets->size(); i++)
        {
            AppendToLog("Retransmission: ");
            PrintPacketInfo(retransmitPackets->at(i));
            WriteUDP(retransmitPackets->at(i));
            sem2.release();
        }
    }
 }

 bool MainWindow::isDuplicatePacket(packet p)
 {
     for (int i = 0; i < receivedDataPackets->size(); i++)
     {
         if (p.SeqNum == receivedDataPackets->at(i).SeqNum)
         {
            return true;
         }
     }

     return false;
 }

 void MainWindow::eot_ack_timeout()
 {
    if (reTransmitCount < max_retransmissions)
    {
        reTransmitCount++;

    }
 }
