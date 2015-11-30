#include "sendthread.h"
#include <Windows.h>

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
        if (true)
        {
            float windowSize = file.size()/DATA_SIZE;
            window->BuildPacket(dgram, 0, seqNum, (int)windowSize, DATA_PACKET, RECEIVER_PORT, temp, (char*)RECV_ADDR);

            if(window->currentPacketWindow->size() <= WINDOW_SIZE
                    && (file.size() - i) >= DATA_SIZE)
            {
                window->currentPacketWindow->push_back(dgram);
            }
            else
            {
                // Write the datagrams
                for (int j = 0; j < window->currentPacketWindow->size(); j++)
                {
                    window->WriteUDP(window->currentPacketWindow->at(j));
                }

                window->currentPacketWindow->clear();
            }

            i += DATA_SIZE;
            seqNum++;
            file.seek(i);
            memset(temp, 0, sizeof(temp));
        }
    }
}
