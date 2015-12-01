#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define NETWORK_PORT 7003
#define RECEIVER_PORT 7001
#define TRANSMIT_PORT 7002
#define CONTROL_PACKET 001
#define DATA_PACKET 002
#define EOT_PACKET 003
#define EOT_ACK_PACKET 004
#define DATA_SIZE 256
#define MAX_RETRANSMISSIONS 4
#define WINDOW_SIZE 5
#define TIMEOUT 500


#include <QMainWindow>
#include <QtNetwork>
#include <QTimer>
#include <QSettings>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    const char *RECV_ADDR = "127.0.0.1";
    const char *NETWORK_ADDR = "192.168.1.72";
    const char *TRANSMIT_ADDR = "192.168.1.69";

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

    QVector<packet> *currentPacketWindow;
    bool allPacketsAckd;
    packet sentPacket;
    bool lastPacket;

    QString recv_addr;
    QString network_addr;
    QString transmit_addr;
    int timeout;
    int window_size;
    int max_retransmissions;
    int network_port;
    int receiver_port;
    int transmit_port;

    explicit MainWindow(QWidget *parent = 0);
    void loadFiles();
    void updateFileList();
    void AppendToLog(QString);
    void WriteUDP(packet);
    void ProcessPacket(packet);
    void BuildPacket(packet &p, int ack, int seq, int win, int type, int destPort, char* data, char* destAddr);
    void PrintPacketInfo(packet p);
    bool isDuplicatePacket(packet p);
    ~MainWindow();

signals:
    void valueChanged(QString);

private slots:
    void readtxDatagrams();
    void on_listView_doubleClicked(const QModelIndex &index);
    void timeoutEvent();
    void eot_ack_timeout();

private:
    Ui::MainWindow *ui;
    QUdpSocket *tx_socket;
    QUdpSocket *rx_socket;
    QStringList _fileList;
    QTimer *timer;
    QTimer *EOTAckTimer;
    int previous_ack;
    QVector<packet> *receivedControlPackets;
    QVector<packet> *retransmitPackets;
    QVector<packet> *receivedDataPackets;
    int reTransmitCount;
};

#endif // MAINWINDOW_H
