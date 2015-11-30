#ifndef LISTENTHREAD_H
#define LISTENTHREAD_H

#include <QThread>
#include "mainwindow.h"

class ListenThread : public QThread
{
    Q_OBJECT
public:
    ListenThread(QObject *parent);
    void run();

private:
    QUdpSocket *rx_socket;
    MainWindow *window;

signals:

private slots:
    void readrxDatagrams();
};

#endif // LISTENTHREAD_H
