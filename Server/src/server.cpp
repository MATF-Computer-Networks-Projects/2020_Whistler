#include "headers/server.h"

#include <QDebug>
#include <QTextStream>

#include "headers/client.h"

Server::Server(QObject *parent) : QTcpServer(parent) {}

bool Server::startServer(quint16 port) {
  // listening to all incoming connections
  return listen(QHostAddress::Any, port);
}

void Server::incomingConnection(qintptr socketDesc) {
  qDebug() << "Connected client with socket descriptor:" << socketDesc;
  auto socket = new Client(socketDesc, this);
  allClients << socket;

  for (const auto &c : allClients) {
    // let every other client know new client has connected
    if (c != socket) {
      QTextStream stream(c);
      stream << "Connected client: " << socket->socketDescriptor();
      c->flush();
    }
  }
  // when any client writes message, server sends that message to everyone else
  // (with broadcastAll)
  connect(socket, &Client::readSignal, this, &Server::broadcastAll);
  connect(socket, &Client::stateChangedSignal, this,
          &Server::clientDisconnected);
}

void Server::clientDisconnected(Client *client, int state) {
  qDebug() << "Client with socket descriptor:" << client->socketDesc()
           << "changed state";
  if (state == QTcpSocket::UnconnectedState) {
    qDebug() << "Disconnected client with socket descriptor"
             << client->socketDesc();
    allClients.removeOne(client);

    // notice everyone that client has disconnected
    for (const auto &c : allClients) {
      QTextStream stream(c);
      stream << "Server: client: " << client->socketDesc()
             << " has disconnected";
      c->flush();
    }
  }
}

void Server::broadcastAll(Client *client) {
  qDebug() << "Read from client:" << client;
  QTextStream streamFrom(client);
  auto text = streamFrom.readAll();

  for (const auto &c : allClients) {
    if (c != client) {
      QTextStream stream(c);
      stream << text;
      c->flush();
    }
  }
}
