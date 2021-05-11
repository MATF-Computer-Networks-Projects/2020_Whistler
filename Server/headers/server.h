#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QSqlDatabase>
#include <QTcpServer>
#include <QWidget>
#include <QtSql>

class Client;

class Server : public QTcpServer {
public:
  Server(QObject *parent = nullptr);
  bool startServer(quint16 port);
  void incomingConnection(qintptr socketDesc);

private:
  QList<Client *> allClients;

  QString dbHostname;
  QString dbUsername;
  QString dbPassword;
  QString dbDatabaseName;
  QString dbPort;
  QSqlDatabase db;

private slots:
  void broadcastAll(Client *client);
  void clientDisconnected(Client *stream, int state);
};

#endif // SERVER_H
