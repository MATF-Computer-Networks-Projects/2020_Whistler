#include "headers/server.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtSql>
#include <iostream>

#include "headers/client.h"
#include "headers/simplecrypt.h"

Server::Server(QObject *parent) : QTcpServer(parent) {}

bool Server::startServer(quint16 port) {

  //  QFile databaseInfo("db.txt");
  //  if (databaseInfo.open(QFile::ReadOnly | QFile::Text)) {
  //    QTextStream in(&databaseInfo);
  //    QString line;

  //    line = in.readLine();

  //    QList<QString> info = line.split("$$$");
  //    dbHostname = info[0];
  //    dbUsername = info[1];
  //    dbPassword = info[2];
  //    dbDatabaseName = info[3];
  //    dbPort = info[4];
  //  } else {
  //    std::cout << "Error while opening file db.txt";
  //        return -1;
  //  }

  //  db = QSqlDatabase::addDatabase("QSQLITE");
  //  db.setHostName(dbHostname);
  //  db.setUserName(dbUsername);
  //  db.setPassword(dbPassword);
  //  db.setDatabaseName(dbDatabaseName);
  //  db.setPort(dbPort.toInt());

  db = QSqlDatabase::addDatabase("QMYSQL");
  db.setHostName("b94tnhh8w1kwl9kczzst-mysql.services.clever-cloud.com");
  db.setUserName("uiesn64ti0hf90xg");
  db.setPassword("lY957lzqrPJVapv0sME9");
  db.setDatabaseName("b94tnhh8w1kwl9kczzst");
  db.setPort(3306);

  if (db.open()) {

    // unique id (no need to check or use select last id from db or something)
    //    quint64 id = QDateTime::currentMSecsSinceEpoch();
    //    QString hashedPassword;
    //    SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
    //    hashedPassword = crypto.encryptToString(password);

    //    query->prepare("INSERT INTO Accounts "
    //                    "VALUES (:id,:username,:password)");
    //    query.bindValue(":id", id);
    //    query.bindValue(":username", username);
    //    query.bindValue(":password", hashedPassword);

    QSqlQuery query(db);
    query.exec("INSERT INTO Accounts "
               "VALUES ('12345','username','password')");

    query.exec("SELECT id FROM Accounts");
    qDebug() << "pre";
    while (query.next()) {
      qDebug() << query.value(0).toInt();
    }
    qDebug() << "posle";

    if (db.isOpen()) {
      qDebug("radi ovde");
    }

    qDebug() << "radi";

  } else {
    qDebug() << "ne radi";
  }

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
