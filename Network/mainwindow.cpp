#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::AnyIPv4, NETWORK_PORT);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));
    connect(this, SIGNAL(valueChanged(QString)), ui->textBrowser, SLOT(append(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::AppendToLog(QString s)
{
    emit valueChanged(s);
}

void MainWindow::readDatagrams()
{
    packet p;

    while (socket->hasPendingDatagrams())
    {
        socket->readDatagram((char*)&p, sizeof(p));
        ProcessPacket(p);
    }
}

void MainWindow::ProcessPacket(packet p)
{
    PrintPacketInfo(p);
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
                                 "Destination: %5\n")
            .arg(pType,
                 QString::number(p.SeqNum),
                 QString::number(p.WindowSize),
                 QString::number(p.AckNum),
                 p.dest_addr);

    AppendToLog(packetInfo);
   // AppendToLog(p.data);
}
