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

    const char *RECV_ADDR = "localhost";

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
    void loadFiles();
    void updateFileList();
    void AppendToLog(QString);
    void WriteUDP(packet);
    void ProcessPacket(packet);
    void BuildPacket(packet &p, int ack, int seq, int win, int type, char* data, char* destAddr);
    ~MainWindow();

signals:
    void valueChanged(QString);

private slots:
    void readDatagrams();
    void on_listView_doubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    QUdpSocket *socket;
    QStringList _fileList;

};

#endif // MAINWINDOW_H