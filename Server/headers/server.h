#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QWidget>

class Client;

class Server : public QTcpServer {
public:
  Server(QObject *parent = nullptr);
  bool startServer(quint16 port);
  void clientConnected(qintptr socketDesc);

private:
  QList<Client *> allClients;

private slots:
  void broadcastAll(Client *client);
  void clientDisconnected(Client *stream, int state);
};

#endif // SERVER_H
