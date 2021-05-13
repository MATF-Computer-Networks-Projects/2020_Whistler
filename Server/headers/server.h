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
  ~Server();
  bool startServer(quint16 port);
  void incomingConnection(qintptr socketDesc);
  int port = 12345;

private:
  QList<Client *> allClients;
  QList<Client *> loggedIn;

  QMap<int, Client *> clients;

  QString dbHostname;
  QString dbUsername;
  QString dbPassword;
  QString dbDatabaseName;
  QString dbPort;
  QSqlDatabase db;

  QString signupCheckString = "0xa$124432";
  QString loginCheckString = "0xa$124jk2";
  QString logoutString = "0xa$1212k2";
  QString onlineString = "0xa$121422";
  QString offlineString = "0xa$121322";
  QString changePasswordString = "0xa$112322";
  QString allOnlineString = "0xa$111432";
  QString separator = "$$$";

  quint64 hashingValue = Q_UINT64_C(0x0c2ad4a4acb9f023);

private slots:
  void serveClient(Client *client);
  void clientDisconnected(Client *stream, int state);
};

#endif // SERVER_H
