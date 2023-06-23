#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QMap>

QMap<QString, int> wins;
QMap<QString, int> losses;

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
    server_status = 0;
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "Server is not started";
    } else {
        server_status = 1;
        qDebug() << "Server is started";
    }

    initializeStatsFile();
}

void MyTcpServer::initializeStatsFile()
{
    QFile file("stats.txt");

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);

        // Запись заголовка в файл статистики
        stream << "User Statistics:\r\n";
        stream << "-----------------\r\n";

        file.close();
    }
}

void MyTcpServer::initializeUserStats(const QString& login)
{
    wins[login] = 0;
    losses[login] = 0;
}

void MyTcpServer::slotNewConnection()
{
    if (server_status == 1) {
        mTcpSocket = mTcpServer->nextPendingConnection();
        mTcpSocket->write("connected\r\n");
        connect(mTcpSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
        connect(mTcpSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
    }
}

void MyTcpServer::slotServerRead()
{
    while (mTcpSocket->canReadLine()) {
        QByteArray array = mTcpSocket->readLine().trimmed();
        QString request = QString::fromUtf8(array);

        if (request.startsWith("start&login//")) {
            QString login = request.mid(13);
            addWaitingClient(login);
        } else if (request == "break") {
            removeClient(mTcpSocket);
        } else if (request == "stats") {
            sendActiveClients();
        } else if (request.startsWith("choice&number//")) {
            QString choiceString = request.mid(14);
            int choice = choiceString.toInt();
            makeMove(choice);
        }
    }
}

void MyTcpServer::slotClientDisconnected()
{
    removeClient(mTcpSocket);
}

void MyTcpServer::addWaitingClient(const QString& login)
{
    if (waitingClients.size() < 5) {
        waitingClients.insert(mTcpSocket, login);
        mTcpSocket->write("waiting\r\n");

        if (waitingClients.size() == 5) {
            sendToAllClients("make_move\r\n");
        }
    } else {
        mTcpSocket->write("Game is full\r\n");
        mTcpSocket->disconnectFromHost();
    }
}

void MyTcpServer::removeClient(QTcpSocket* socket)
{
    if (waitingClients.contains(socket)) {
        waitingClients.remove(socket);
    } else if (activeClients.contains(socket)) {
        QString login = activeClients.value(socket);
        activeClients.remove(socket);
        updateStats(login); // Исправлен вызов функции updateStats()
    }

    socket->close();
    socket->deleteLater();
}

void MyTcpServer::sendActiveClients()
{
    QString activeClientsList = "Active clients:\r\n";

    for (const QString& login : activeClients.values()) {
        activeClientsList += login + "\r\n";
    }

    mTcpSocket->write(activeClientsList.toUtf8());
}

void MyTcpServer::makeMove(int choice)
{
    if (activeClients.contains(mTcpSocket)) {
        int maxChoice = choice;
        bool isMaxChoice = true;

        for (QTcpSocket* socket : activeClients.keys()) {
            if (socket != mTcpSocket) {
                int clientChoice = rand() % 100 + 1;
                if (clientChoice > maxChoice) {
                    maxChoice = clientChoice;
                    isMaxChoice = false;
                }
            }
        }

        QString result;

        if (isMaxChoice) {
            result = "win\r\n";
            updateStats(activeClients.value(mTcpSocket), true); // Исправлен вызов функции updateStats()
        } else {
            result = "lost\r\n";
            updateStats(activeClients.value(mTcpSocket), false); // Исправлен вызов функции updateStats()
        }

        sendToAllClients(result);
        removeClient(mTcpSocket);
    }
}

void MyTcpServer::sendToAllClients(const QString& message)
{
    for (QTcpSocket* socket : activeClients.keys()) {
        socket->write(message.toUtf8());
    }
}

void MyTcpServer::updateStats(const QString& login)
{
    updateStats(login, true); // Передано дополнительное значение по умолчанию
}

void MyTcpServer::updateStats(const QString& login, bool isWin)
{
    if (isWin) {
        wins[login]++;
    } else {
        losses[login]++;
    }

    QFile file("stats.txt");

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "Login: " << login << ", Wins: " << wins[login] << ", Losses: " << losses[login] << "\r\n";
        file.close();
    }
}
