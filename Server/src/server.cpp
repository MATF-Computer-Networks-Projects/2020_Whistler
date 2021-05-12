#include "headers/server.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtSql>
#include <iostream>

#include "headers/client.h"
#include "headers/simplecrypt.h"

Server::Server(QObject *parent) : QTcpServer(parent) {}
Server::~Server() { db.close(); }

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

    //    query.prepare("INSERT INTO Accounts "
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
  //  allClients << socket;

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
  connect(socket, &Client::readSignal, this, &Server::serveClient);
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

void Server::serveClient(Client *client) {
  qDebug() << "Read from client:" << client;
  QTextStream streamFrom(client);
  auto message = streamFrom.readAll();

  // signup check
  if (message.startsWith(signupCheckString)) {
    qDebug() << "Checking account on signup";
    QList<QString> userInfo = message.split(separator);
    QString username = userInfo[1];
    QString password = userInfo[2];
    qDebug() << "username" << username;

    QSqlQuery query(db);

    query.prepare("SELECT * FROM Accounts where username=(:username)");

    query.bindValue(":username", username);

    query.exec();
    int numRows;

    // First way
    if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
      qDebug() << "prvo";
      numRows = query.size();
      //      if (query.isActive()) {
      //        qDebug() << "active";
      //      } else {
      //        qDebug() << "not active";
      //      }
      //      while (query.next()) {
      //        int id = query.value(0).toInt();
      //        QString username = query.value(1).toString();
      //        QString password = query.value(2).toString();
      //        qDebug() << id << username << password;
      //      }
    } else {
      qDebug() << "drugo";
      // this can be very slow
      query.at();
      numRows = query.at() + 1;
    }

    // if there is no rows with that username we add user to database
    if (numRows == 0) {
      streamFrom << "Successful";

      // unique id (no need to check or use select last id from db or something)
      quint64 id = QDateTime::currentMSecsSinceEpoch();
      QString hashedPassword;
      SimpleCrypt crypto(hashingValue);
      hashedPassword = crypto.encryptToString(password);

      // add new user to database
      query.prepare("INSERT INTO Accounts "
                    "VALUES (:id,:username,:password)");
      query.bindValue(":id", id);
      query.bindValue(":username", username);
      query.bindValue(":password", hashedPassword);
      query.exec();

    } else {
      streamFrom << "Unsuccessful";
    }

    // // Second way
    //    query.next();
    //    if (query.isValid()) {
    //      qDebug() << query.isValid();
    //      // there is instance in database with that username
    //      int id = query.value(0).toInt();
    //      QString username = query.value(1).toString();
    //      QString password = query.value(2).toString();
    //      qDebug() << id << username << password;
    //      streamFrom << "Unsuccessful";
    //      qDebug() << "Unsuccessful";
    //    } else {
    //      streamFrom << "Successful";
    //      qDebug() << "Successful";
    //    }

    client->flush();

    // login check
  } else if (message.startsWith(loginCheckString)) {
    qDebug() << "Checking account on login";
    QList<QString> userInfo = message.split(separator);
    QString username = userInfo[1];
    QString password = userInfo[2];
    qDebug() << "username" << username;

    QSqlQuery query(db);

    query.prepare("SELECT * FROM Accounts where username=(:username)");

    query.bindValue(":username", username);

    query.exec();
    int numRows;

    // First way
    if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
      qDebug() << "prvo";
      numRows = query.size();

    } else {
      qDebug() << "drugo";
      // this can be very slow
      query.at();
      numRows = query.at() + 1;
    }

    // if there is no rows with that username then that is wrong username
    if (numRows == 0) {
      streamFrom << "Unsuccessful user doesnt exist";
      qDebug() << "Unsuccessful user doesnt exist";

    } else {
      query.next();
      if (query.isValid()) {
        qDebug() << query.isValid();
        // there is instance in database with that username
        int idDb = query.value(0).toInt();
        QString usernameDb = query.value(1).toString();
        QString passwordDb = query.value(2).toString();
        qDebug() << idDb << usernameDb << passwordDb;

        // checking if password is correct
        QString unhashedPassword;
        SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
        unhashedPassword = crypto.decryptToString(passwordDb);

        qDebug() << "unhashed password" << unhashedPassword;

        if (unhashedPassword == password) {
          streamFrom << "Successful";
          qDebug() << "Successful";
          // add socket of client to allClients
          allClients << client;
        } else {

          streamFrom << "Unsuccessful wrong password";
          qDebug() << "Unsuccessful wrong password";
        }
      }

      client->flush();
    }

    // basic message
  } else {
    QList<QString> messageData = message.split(separator);
    QString username = messageData[0];
    QString messageText = messageData[1];

    // sending message to all clients except the one who sent it
    for (const auto &c : allClients) {
      if (c != client) {
        QTextStream stream(c);
        stream << username << " sent: " << messageText;
        c->flush();
      }
    }
  }
}
