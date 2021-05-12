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
  int port = 12345;

private:
  QList<Client *> allClients;

  QString dbHostname;
  QString dbUsername;
  QString dbPassword;
  QString dbDatabaseName;
  QString dbPort;
  QSqlDatabase db;

  QString signupCheckString = "0xa$124432";
  QString separator = "$$$";

private slots:
  void serveClient(Client *client);
  void clientDisconnected(Client *stream, int state);
};

#endif // SERVER_H
