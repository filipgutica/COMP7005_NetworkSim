#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define NETWORK_PORT 7003
#define RECEIVER_PORT 7001
#define TRANSMIT_PORT 7002
#define CONTROL_PACKET 001
#define DATA_PACKET 002
#define DATA_SIZE 256
#define WINDOW_SIZE 10
#define BIT_ERROR_RATE 10

#include <QMainWindow>
#include <QtNetwork>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    struct packet
    {
        int PacketType;
        int SeqNum;
        char data[DATA_SIZE];
        int WindowSize;
        int AckNum;
        char dest_addr[32];
        int dest_port;
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void AppendToLog(QString);
    QUdpSocket *socket;
    QUdpSocket *tx_socket;
    void ProcessPacket(packet);
    void PrintPacketInfo(packet);
    void WriteUDP(packet p);

signals:
    void valueChanged(QString);

private slots:
    void readDatagrams();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
