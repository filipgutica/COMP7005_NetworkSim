#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <Windows.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listView->setEditTriggers(QListView::NoEditTriggers);
    loadFiles();
    updateFileList();

    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::AnyIPv4, TRANSMIT_PORT);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));

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
    char temp[DATA_SIZE];
    int i =0;
    int seqNum = 0;
    packet *dgram = new packet();
    QFile file(index.data().toString());

    //open the file for reading
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "cannot find file";
        return;
    }

    while ((file.read(temp, DATA_SIZE)))
    {
        BuildPacket(*dgram, 0, seqNum, file.size(), DATA_PACKET, temp, (char*)RECV_ADDR);

        // Write the datagram
        WriteUDP(*dgram);

        i += DATA_SIZE;
        seqNum++;
        file.seek(i);
        memset(temp, 0, sizeof(temp));
    }

}

void MainWindow::readDatagrams()
{
    packet packet;

    while (socket->hasPendingDatagrams())
    {
        socket->readDatagram((char*)&packet, sizeof(packet));
    }
}

void MainWindow::WriteUDP(packet p)
{
    socket->writeDatagram( (char*)&p, sizeof(p), QHostAddress::LocalHost, NETWORK_PORT);
}


void MainWindow::ProcessPacket(packet p)
{
    switch (p.PacketType)
    {
        case CONTROL_PACKET:

            break;
        case DATA_PACKET:

            break;
        default:
            break;
    }
}

 void MainWindow::BuildPacket(packet &p, int ack, int seq, int win, int type, char* data, char* destAddr)
 {
     p.AckNum = ack;
     p.PacketType = type;
     sprintf(p.data, "%s", data);
     p.SeqNum = seq;
     p.WindowSize = win;
     sprintf(p.dest_addr, "%s", destAddr);
 }
