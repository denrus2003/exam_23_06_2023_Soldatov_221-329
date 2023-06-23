#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>

class MyTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();
public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();
private:
    QTcpServer* mTcpServer;
    QTcpSocket* mTcpSocket;
    QMap<QTcpSocket*, QString> waitingClients;
    QMap<QTcpSocket*, QString> activeClients;
    int server_status;
    void addWaitingClient(const QString& login);
    void removeClient(QTcpSocket* socket);
    void sendActiveClients();
    void makeMove(int choice);
    void sendToAllClients(const QString& message);
    void updateStats(const QString& login);
    void updateStats(const QString& login, bool isWin);
    void initializeStatsFile();
    void initializeUserStats(const QString& login);
};

#endif // MYSERVER_H
