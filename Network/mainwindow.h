#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define NETWORK_PORT 7003
#define RECEIVER_PORT 7001
#define TRANSMIT_PORT 7002
#define CONTROL_PACKET 001
#define DATA_PACKET 002
#define DATA_SIZE 256
#define WINDOW_SIZE 10

#include <QMainWindow>
#include <QtNetwork>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct packet
    {
        int PacketType;
        int SeqNum;
        char data[DATA_SIZE];
        int WindowSize;
        int AckNum;
        char dest_addr[32];
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void AppendToLog(QString);
    QUdpSocket *socket;
    void ProcessPacket(packet);
    void PrintPacketInfo(packet);

signals:
    void valueChanged(QString);

private slots:
    void readDatagrams();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
