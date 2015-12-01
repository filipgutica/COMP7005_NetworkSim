#include "sendthread.h"
#include "globals.h"
#include <Windows.h>


QSemaphore sem2(WINDOW_SIZE+1);

SendThread::SendThread(QObject *parent)
{
    window = (MainWindow*)parent;
}

void SendThread::setData(QString fName)
{
    fileToSend = fName;
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

        float windowSize = file.size()/DATA_SIZE;
        window->BuildPacket(dgram, 0, seqNum, (int)windowSize, DATA_PACKET, RECEIVER_PORT, temp, (char*)RECV_ADDR);
        window->sentPacket = dgram;

        // Write the datagrams
        sem2.acquire();
        window->currentPacketWindow->push_back(dgram);

      //  window->WriteUDP(dgram);

        if (window->currentPacketWindow->size() >= WINDOW_SIZE || (file.size() - i) <= DATA_SIZE)
        {
            for (int i = 0; i < window->currentPacketWindow->size(); i++)
            {
                window->WriteUDP(window->currentPacketWindow->at(i));
            }

        }

        if ((file.size() - i) <= DATA_SIZE)
            window->lastPacket = true;

       // sem1.release();

        i += DATA_SIZE;
        seqNum++;
        file.seek(i);
        memset(temp, 0, sizeof(temp));


    }
}
