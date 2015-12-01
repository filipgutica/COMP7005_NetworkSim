#include "sendthread.h"
#include "globals.h"
#include <Windows.h>



SendThread::SendThread(QObject *parent)
{
    window = (MainWindow*)parent;
}

void SendThread::setData(QString fName, QString addr)
{
    fileToSend = fName;
    recv_addr = addr;
}

void SendThread::run()
{

    char temp[DATA_SIZE];
    int i =0;
    int seqNum = 0;
    MainWindow::packet dgram;
    QFile file(fileToSend);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "cannot find file";
        return;
    }


    while ((file.read(temp, DATA_SIZE)))
    {

        float windowSize = window->window_size - window->currentPacketWindow->size();
        window->BuildPacket(dgram, 0, seqNum, (int)windowSize, DATA_PACKET, window->receiver_port, temp, recv_addr.toUtf8().data());
        window->sentPacket = dgram;

        // Write the datagrams
        sem2.acquire();
        window->currentPacketWindow->push_back(dgram);

      //  window->WriteUDP(dgram);

        if (window->currentPacketWindow->size() == window->window_size)
        {
            for (int i = 0; i < window->currentPacketWindow->size(); i++)
            {
                window->WriteUDP(window->currentPacketWindow->at(i));
            }

        }

        if ((file.size() - i) <= DATA_SIZE)
        {
            window->BuildPacket(dgram, 0, 0, 0, EOT_PACKET, window->receiver_port, (char*)"EOT", recv_addr.toUtf8().data());
            window->currentPacketWindow->push_back(dgram);
            for (int i = 0; i < window->currentPacketWindow->size(); i++)
            {
                window->WriteUDP(window->currentPacketWindow->at(i));
            }
            window->lastPacket = true;
        }

      //  sem1.release();

        i += DATA_SIZE;
        seqNum++;
        file.seek(i);
        memset(temp, 0, sizeof(temp));


    }
}
