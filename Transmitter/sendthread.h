#ifndef SENDTHREAD_H
#define SENDTHREAD_H


#include <QThread>
#include "mainwindow.h"

class SendThread : public QThread
{
    const char *RECV_ADDR = "127.0.0.1";

    Q_OBJECT
public:
    SendThread(QObject *parent);
    void setData(QString, QString);
    void run();

private:
    QUdpSocket *tx_socket;
    QString fileToSend;
    MainWindow *window;
    QString recv_addr;
signals:

public slots:
};

#endif // SENDTHREAD_H
